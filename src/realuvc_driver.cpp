#include <librealuvc/realuvc_driver.h>
#include <condition_variable>
#include <chrono>

#if 0
#define D(...) { printf("DEBUG[%s,%d] ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#define D(...) { }
#endif

namespace librealuvc {

namespace {
  
// We have a single DevMatAllocator

class DevMatAllocator : public cv::MatAllocator {
 public:
  DevMatAllocator() { }
  
  virtual ~DevMatAllocator() { }
  
  virtual cv::UMatData* allocate(
    int dims, const int* sizes, int type, void* data,
    size_t* step, cv::AccessFlag flags, cv::UMatUsageFlags usage
  ) const {
    auto alloc = cv::Mat::getDefaultAllocator();
    auto result = alloc->allocate(dims, sizes, type, data, step, flags, usage);
    if (result) result->currAllocator = alloc;
    return result;
  }
  
  virtual bool allocate(cv::UMatData* data, cv::AccessFlag flags, cv::UMatUsageFlags usage) const {
    auto alloc = cv::Mat::getDefaultAllocator();
    auto result = alloc->allocate(data, flags, usage);
    if (result) data->currAllocator = alloc;
    return result;
  }
  
  virtual void copy(
    cv::UMatData* src, cv::UMatData* dst, int dims, const size_t sz[],
    const size_t srcofs[], const size_t srcstep[],
    const size_t dstofs[], const size_t dststep[],
    bool sync
  ) const {
    auto alloc = (dst->currAllocator ? dst->currAllocator : cv::Mat::getDefaultAllocator());
    alloc->copy(
      src, dst, dims, sz, srcofs, srcstep, dstofs, dststep, sync
    );
    dst->currAllocator = alloc;
  }
  
  // deallocate() is the only method with non-default behavior
  
  virtual void deallocate(cv::UMatData* data) const {
    DevFrame* f = (DevFrame*)data->handle;
    data->handle = nullptr;
    if (f) delete f;
  }
  
  virtual void download(
    cv::UMatData* data, void* dst, int dims, const size_t sz[],
    const size_t srcofs[], const size_t srcstep[], const size_t dststep[]
  ) const {
    auto alloc = cv::Mat::getDefaultAllocator();
    alloc->download(data, dst, dims, sz, srcofs, srcstep, dststep);
  }
  
  virtual cv::BufferPoolController* getBufferPoolController(const char* id = NULL) const {
    auto alloc = cv::Mat::getDefaultAllocator();
    return alloc->getBufferPoolController();
  }
  
  virtual void map(cv::UMatData* data, cv::AccessFlag flags) const {
    auto alloc = cv::Mat::getDefaultAllocator();
    alloc->map(data, flags);
  }
  
  virtual void unmap(cv::UMatData* data) const {
    // From reading the source code of OpenCV, it turns out that unmap()
    // is the method called when the refcount goes to zero.
    //D("DevMatAllocator::unmap(umatdata %p) DevFrame %p", data, data->handle);
    DevFrame* f = (DevFrame*)data->handle;
    data->handle = nullptr;
    if (f) delete f;
  }

  virtual void upload(
    cv::UMatData* data, const void* src, int dims, const size_t sz[],
    const size_t dstofs[], const size_t dststep[], const size_t srcstep[]
  ) {
    auto alloc = cv::Mat::getDefaultAllocator();
    alloc->upload(data, src, dims, sz, dstofs, dststep, srcstep);
  }
};

DevMatAllocator single_alloc;

} // end anon

// DevFrame methods

DevFrame::DevFrame(
  const stream_profile& profile,
  const frame_object& frame,
  const std::function<void()>& release_func
) :
  cv::UMatData(&single_alloc),
  profile_(profile),
  frame_(frame),
  release_func_(release_func),
  is_released_(false) {
  handle = (void*)this;
}

void DevFrame::release() {
  if (!is_released_) {
    is_released_ = true;
    release_func_();
  }
}
  
DevFrame::~DevFrame() {
  if (!is_released_) release_func_();
}
  
// DevFrameQueue methods

DevFrameQueue::DevFrameQueue(DevFrameFixup fixup, size_t max_size) {
  fixup_ = fixup;
  num_sleepers_ = 0;
  if (max_size < 1) max_size = 1;
  max_size_ = max_size;
  size_ = 0;
  front_ = 0;
  queue_.resize(max_size_);
}
  
DevFrameQueue::~DevFrameQueue() {
  while (size_ > 0) drop_front_locked();
}
  
void DevFrameQueue::drop_front_locked() {
  auto f = queue_[front_];
  queue_[front_] = nullptr;
  front_ = ((front_ + 1) % max_size_);
  --size_;
  delete f;
}
  
void DevFrameQueue::push_back(
  const stream_profile& profile,
  const frame_object& frame,
  const std::function<void()>& release_func
) {
  std::unique_lock<std::mutex> lock(mutex_);
  D("DevFrameQueue::push_back() frame.frame_size %d", (int)frame.frame_size);
  while (size_ >= max_size_) drop_front_locked();
  size_t back = ((front_ + size_) % max_size_);
  queue_[back] = new DevFrame(profile, frame, release_func);
  ++size_;
  if (num_sleepers_ > 0) {
    --num_sleepers_;
    wakeup_.notify_one();
  }
}

void print_mat(const char* what, const cv::Mat& mat) {
  printf("print_mat(%s):\n", what);
  printf("  allocator %p (DevMatAllocator %p)\n", (void*)mat.allocator, (void*)&single_alloc);
  printf("  cols %d\n", mat.cols);
  printf("  data %p\n", (void*)mat.data);
  printf("  dims %d\n", mat.dims);
  printf("  flags 0x%08x\n", mat.flags);
  printf("  rows %d\n", mat.rows);
  for (int j = 0; j < mat.size.dims(); ++j) {
    printf("  size[%d] %ld\n", j, (long)mat.size[j]);
  }
  printf("  step %ld\n", (long)mat.step);
  printf("  umatdata %p\n", (void*)mat.u);
  fflush(stdout);
  auto u = mat.u;
  if (!u) return;
  printf("    allocatorFlags_ 0x%08x\n", u->allocatorFlags_);
  printf("    currAllocator %p\n", (void*)u->currAllocator);
  printf("    data %p\n", (void*)u->data);
  printf("    flags 0x%08x\n", (int)u->flags);
  printf("    handle %p\n", (void*)u->handle);
  printf("    mapcount %d\n", u->mapcount);
  printf("    origdata %p\n", (void*)u->origdata);
  printf("    origUMatData %p\n", (void*)u->originalUMatData);
  printf("    prevAllocator %p\n", (void*)u->prevAllocator);
  printf("    refcount %d\n", u->refcount);
  printf("    size %ld\n", (long)u->size);
  printf("    urefcount %d\n", u->urefcount);
  printf("    userdata %p\n", (void*)u->userdata);
  printf("\n");
  fflush(stdout);
}
  
bool DevFrameQueue::pop_front(ru_time_t& ts, cv::Mat& mat) {
  std::unique_lock<std::mutex> lock(mutex_);

  while (size_ <= 0) {
    ++num_sleepers_;
    if (wakeup_.wait_for(lock, std::chrono::milliseconds(250)) == std::cv_status::timeout) {
      D("pop_front - wait for messages timed out");
      return false;
    }
  }

  size_t front = front_;
  DevFrame* f = queue_[front];
  queue_[front] = nullptr;
  front_ = ((front + 1) % max_size_);
  --size_;
  ts = f->frame_.backend_time;
  cv::UMatData* data = f;
  D("pop_front DevFrame %p frame_size %d", (void*)f, (int)f->frame_.frame_size);
  cv::Mat m(0, 0, CV_8UC1);
  m.allocator = &single_alloc;
  m.cols = f->profile_.width;
  m.rows = f->profile_.height;
  m.data = (uchar*)f->frame_.pixels;
  m.dims = 2;
  m.step.p[0] = (m.cols * sizeof(uint8_t));
  m.step.p[1] = sizeof(uint8_t);
  D("frame data %p data+size %p", m.data, m.data+f->frame_.frame_size);
  // Leap Motion devices pretend to be giving frames in YUY2 format
  // (4 bytes for 2 pixels), but it's really 8bit grayscale with
  // each row containing both the L and R rows.
  int fourcc_YUY2 = 0x59555932;
  switch (fixup_) {
    case FIXUP_NORMAL:
      // The frame is just fine, do nothing
      // WARNING: this works for I420 format which starts with complete Y-plane
      break;
    case FIXUP_GRAY8_PIX_L_PIX_R: {
      // We need to rearrange the data within each row
      int halfcols = m.cols;
      m.cols *= 2;
      std::vector<uchar> halfrow(halfcols);
      uchar* src = m.data;
      for (int row = 0; row < m.rows; ++row) {
        uchar* final_R = (src + halfcols);
        uchar* srclim = (src + m.cols);
        uchar* dst_L = src;
        uchar* dst_R = &halfrow[0];
        for (; src < srclim; src += 2) {
          *dst_L++ = src[0];
          *dst_R++ = src[1];
        }
        memcpy(final_R, &halfrow[0], halfcols*sizeof(uchar));
      }
      break;
    }
    case FIXUP_GRAY8_ROW_L_ROW_R:
      // The data layout is fine, but it's 8-bit pixels not 16-bit
      m.cols *= 2;
      break;
  }
  m.rows = f->profile_.height;
  m.step = m.cols * sizeof(uint8_t);
  m.u = data;
  data->data = m.data;
  data->refcount = 1;
  data->size = 1;
  mat = m;
  
  return true;
}

} // end librealuvc
