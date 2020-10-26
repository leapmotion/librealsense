#ifndef REALUVC_REALUVC_DRIVER_H
#define REALUVC_REALUVC_DRIVER_H 1
//
// Support for loadable camera-device drivers
//
// Copyright (c) Leap Motion Corp 2019.  All rights reserved.
//
#include <librealuvc/realuvc.h>
#include <opencv2/core.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace librealuvc {

using std::shared_ptr;
using std::vector;

// Support for managing buffer lifetime without copying
//
// The cv::Mat's populated by librealuvc may refer to buffers
// from a lower-level video framework.

class DevFrame : public cv::UMatData {
 public:
  stream_profile profile_;
  frame_object frame_;
 private:
  std::function<void()> release_func_;
  bool is_released_;
 
 public:
  DevFrame(
    const stream_profile& profile,
    const frame_object& frame,
    const std::function<void()>& release_func
  );
  
  void release();
  
  ~DevFrame();
};

class DevFrameQueue {
 private:
  std::mutex mutex_;
  std::condition_variable wakeup_;
  int num_sleepers_;
  DevFrameFixup fixup_;
  size_t max_size_;
  size_t size_;
  size_t front_;
  vector<DevFrame*> queue_;
 
 public:
  DevFrameQueue(DevFrameFixup fixup, size_t max_size = 1);
  
  ~DevFrameQueue();
  
  void drop_front_locked();
  
  void push_back(
    const stream_profile& profile,
    const frame_object& frame,
    const std::function<void()>& release_func
  );
  
  bool pop_front(ru_time_t& ts, cv::Mat& mat);  
};

} // end librealuvc

#endif
