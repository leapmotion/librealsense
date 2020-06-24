// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#ifndef LIBREALUVC_RU_COMMON_H
#define LIBREALUVC_RU_COMMON_H 1

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace librealuvc {

using std::shared_ptr;
using std::string;
using std::vector;

typedef double ru_time_t; // Timestamp in milliseconds

typedef ru_time_t rs2_time_t;

#ifdef linux 
typedef uint8_t byte;
#endif

struct frame_object {
  size_t      frame_size;
  uint8_t     metadata_size;
  const void* pixels;
  const void* metadata;
  ru_time_t   backend_time;
};

#define RU_FOURCC(c3, c2, c1, c0) ( \
  (((int32_t)(c3)) << 24) | \
  (((int32_t)(c2)) << 16) | \
  (((int32_t)(c1)) <<  8) | \
  ((int32_t)c0) \
)

#define RU_FOURCC_YUY2 RU_FOURCC('Y', 'U', 'Y', '2')
#define RU_FOURCC_NV12 RU_FOURCC('N', 'V', '1', '2')

typedef std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> stream_profile_tuple;

class stream_profile {
 public:
  uint32_t width;
  uint32_t height;
  uint32_t fps;
  uint32_t format;
  
 public:
  bool operator==(const stream_profile& b) const;
  operator stream_profile_tuple() const;
  string to_string() const;
  operator string() const { return this->to_string(); }
};

// The third arg to frame_callback is a func to release the frame's buffer
// This allows fast processing to be done without copying the data.

typedef std::function<
  void(const stream_profile&, const frame_object&, std::function<void()>)
> frame_callback;

class notification {
  // FIXME
};

enum usb_spec : uint16_t {
  usb_undefined   = 0,
  usb1_type       = 0x0100,
  usb1_1_type     = 0x0110,
  usb2_type       = 0x0200,
  usb2_1_type     = 0x0210,
  usb3_type       = 0x0300,
  usb3_1_type     = 0x0310,
  usb3_2_type     = 0x0320,
};

extern const std::map<usb_spec, string> usb_spec_names;

} // end librealuvc

#endif
