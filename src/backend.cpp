// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#if defined(RS2_USE_LIBUVC_BACKEND) && !defined(RS2_USE_WMF_BACKEND) && !defined(RS2_USE_V4L2_BACKEND) && !defined(RS2_USE_WINUSB_UVC_BACKEND)  && !defined(RS2_USE_ANDROID_BACKEND)
// UVC support will be provided via libuvc / libusb backend
#elif !defined(RS2_USE_LIBUVC_BACKEND) && defined(RS2_USE_WMF_BACKEND) && !defined(RS2_USE_V4L2_BACKEND) && !defined(RS2_USE_WINUSB_UVC_BACKEND) && !defined(RS2_USE_ANDROID_BACKEND)
// UVC support will be provided via Windows Media Foundation / WinUSB backend
#elif !defined(RS2_USE_LIBUVC_BACKEND) && !defined(RS2_USE_WMF_BACKEND) && !defined(RS2_USE_V4L2_BACKEND) && defined(RS2_USE_WINUSB_UVC_BACKEND) && !defined(RS2_USE_ANDROID_BACKEND)
// UVC support will be provided via WinUSB / WinUSB backend
#elif !defined(RS2_USE_LIBUVC_BACKEND) && !defined(RS2_USE_WMF_BACKEND) && defined(RS2_USE_V4L2_BACKEND) && !defined(RS2_USE_WINUSB_UVC_BACKEND) && !defined(RS2_USE_ANDROID_BACKEND)
// UVC support will be provided via Video 4 Linux 2 / libusb backend
#elif !defined(RS2_USE_LIBUVC_BACKEND) && !defined(RS2_USE_WMF_BACKEND) && !defined(RS2_USE_V4L2_BACKEND) && !defined(RS2_USE_WINUSB_UVC_BACKEND) && defined(RS2_USE_ANDROID_BACKEND)
// UVC support will be provided via Android backend
#else
#error No UVC backend selected. Please #define exactly one of RS2_USE_LIBUVC_BACKEND, RS2_USE_WMF_BACKEND, RS2_USE_WINUSB_UVC_BACKEND or RS2_USE_V4L2_BACKEND
#endif

#include "backend.h"

namespace librealuvc {

using std::shared_ptr;
using std::string;

shared_ptr<backend> create_backend() {
  return platform::create_backend();
}

// control_range is declared in <librealuvc/ru_uvc.h>

control_range::control_range() { }

control_range::control_range(int32_t amin, int32_t amax, int32_t astep, int32_t adef) {
  populate_raw_data(min, amin);
  populate_raw_data(max, amax);
  populate_raw_data(step, astep);
  populate_raw_data(def, adef);
}

control_range::control_range(
  vector<uint8_t> amin,
  vector<uint8_t> amax,
  vector<uint8_t> astep,
  vector<uint8_t> adef
) :
  min(amin),
  max(amax),
  step(astep),
  def(adef) {
}

void control_range::populate_raw_data(std::vector<uint8_t>& vec, int32_t value) {
    vec.resize(sizeof(value));
    auto data = reinterpret_cast<const uint8_t*>(&value);
    std::copy(data, data + sizeof(value), vec.data());
}

double monotonic_to_realtime(double monotonic) {
  using namespace std::chrono;
  auto realtime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  auto time_since_epoch = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
  return monotonic + (realtime - time_since_epoch);
}

} // end librealuvc
