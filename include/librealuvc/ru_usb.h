// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#ifndef LIBREALUVC_RU_USB_H
#define LIBREALUVC_RU_USB_H 1

#include "ru_common.h"

namespace librealuvc {

using std::shared_ptr;
using std::string;
using std::vector;

class command_transfer {
 public:
  virtual ~command_transfer() = default;

  virtual vector<uint8_t> send_receive(
    const vector<uint8_t>& data,
    int timeout_ms = 5000,
    bool require_response = true
  ) = 0;
};

class usb_device : public command_transfer {
 public:
  // interrupt endpoint and any additional USB specific stuff
};

class usb_device_info {
 public:
  string id;
  uint16_t vid;
  uint16_t pid;
  uint16_t mi;
  string unique_id;
  usb_spec conn_spec;
 
 public:
  bool operator==(const usb_device_info& b) const;
  string to_string() const;
  operator string() const { return this->to_string(); }
};

} // end librealuvc

#endif
