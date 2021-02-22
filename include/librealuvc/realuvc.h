/* License: Apache 2.0. See LICENSE file in root directory.
   Copyright(c) 2017 Intel Corporation. All Rights Reserved. */

#ifndef LIBREALUVC_REALUVC_H
#define LIBREALUVC_REALUVC_H 1

#include "ru_common.h"
#include "ru_exception.h"
#include "ru_hid.h"
#include "ru_usb.h"
#include "ru_uvc.h"
#include "ru_videocapture.h"
#include <thread>

// These macro definitions are parsed by config_version.cmake

#define RU_API_MAJOR_VERSION    0
#define RU_API_MINOR_VERSION    1
#define RU_API_PATCH_VERSION    1
#define RU_API_BUILD_VERSION    0

namespace librealuvc {

using std::shared_ptr;
using std::string;
using std::vector;

// Forward declaration of classes

class device_watcher;
class notification;
class time_service;

// backend supports querying devices and creating device-handles

class backend {
 public:
  virtual ~backend() = default;
  
  virtual shared_ptr<device_watcher> create_device_watcher() const = 0;
  virtual shared_ptr<hid_device> create_hid_device(hid_device_info info) const = 0;
  virtual shared_ptr<usb_device> create_usb_device(usb_device_info info) const = 0;
  virtual shared_ptr<uvc_device> create_uvc_device(uvc_device_info info) const = 0;
  virtual shared_ptr<time_service> create_time_service() const = 0;
  
  virtual string get_device_serial(uint16_t vid, uint16_t pid, const std::string& uid) const {
    return "";
  }
  
  virtual vector<hid_device_info> query_hid_devices() const = 0;
  virtual vector<usb_device_info> query_usb_devices() const = 0;
  virtual vector<uvc_device_info> query_uvc_devices() const = 0;  
};

// create_backend() gives the appropriate backend for this platform

std::shared_ptr<backend> create_backend();

class backend_device_group {
 public:
  vector<uvc_device_info> uvc_devices;
  vector<usb_device_info> usb_devices;
  vector<hid_device_info> hid_devices;
 
 public:
  backend_device_group();
  backend_device_group(
    const std::vector<uvc_device_info>& uvc_devs,
    const std::vector<usb_device_info>& usb_devs,
    const std::vector<hid_device_info>& hid_devs
  );

  bool operator==(const backend_device_group& b) const;
  string to_string() const;
  operator string() const { return this->to_string(); }
};

typedef std::function<
  void(backend_device_group old_devs, backend_device_group new_devs)
> device_changed_callback;

class device_watcher {
 public:
  virtual ~device_watcher() = default;
  virtual void start(device_changed_callback callback) = 0;
  virtual void stop() = 0;
};

class time_service {
 public:
  virtual ~time_service() = default;
  virtual ru_time_t get_time() const = 0;
};

class os_time_service : public time_service {
 public:
  virtual ~os_time_service() = default;
  virtual ru_time_t get_time() const;
};
}

#endif
