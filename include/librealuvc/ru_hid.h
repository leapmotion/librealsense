// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#ifndef LIBREALUVC_RU_HID_H
#define LIBREALUVC_RU_HID_H 1

#include "ru_common.h"

namespace librealuvc {

using std::shared_ptr;
using std::string;
using std::vector;

class hid_device_info {
 public:
  string id;
  string vid;
  string pid;
  string unique_id;
  string device_path;
  string serial_number;
  
 public:
  bool operator==(const hid_device_info& b) const;
  string to_string() const;
  operator string() const { return this->to_string(); }
};

struct hid_profile {
  string sensor_name;
  uint32_t frequency;
};

struct hid_sensor {
  string name;
};

#pragma pack(push, 1)
struct hid_sensor_data {
  int16_t x;
  char reserved1[2];
  int16_t y;
  char reserved2[2];
  int16_t z;
  char reserved3[2];
  uint32_t ts_low;
  uint32_t ts_high;
};
#pragma pack(pop)

struct hid_sensor_input {
  uint32_t index;
  string name;
};

struct sensor_data {
  hid_sensor sensor;
  frame_object fo;
};

enum custom_sensor_report_field {
  minimum,
  maximum,
  name,
  size,
  unit_expo,
  units,
  value
};

typedef std::function<void(const sensor_data&)> hid_callback;

class hid_device {
 public:
  virtual ~hid_device() = default;
  virtual void open(const vector<hid_profile>& hid_profiles) = 0;
  virtual void close() = 0;
  virtual void start_capture(hid_callback callback) = 0;
  virtual void stop_capture() = 0;
  virtual vector<hid_sensor> get_sensors() = 0;
  virtual vector<uint8_t> get_custom_report_data(
    const string& custom_sensor_name,
    const string& report_name,
    custom_sensor_report_field report_field
  ) = 0;
};

} // end librealuvc

#endif
