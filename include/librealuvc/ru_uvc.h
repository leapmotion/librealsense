// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#ifndef LIBREALUVC_RU_UVC_H
#define LIBREALUVC_RU_UVC_H 1

#include "ru_common.h"
#include "ru_option.h"

namespace librealuvc {

using std::shared_ptr;
using std::string;
using std::vector;

class control_range {
 public:
  vector<uint8_t> min;
  vector<uint8_t> max;
  vector<uint8_t> step;
  vector<uint8_t> def;
 
 private:
  static void populate_raw_data(vector<uint8_t>& vec, int32_t val);
 
 public:
  control_range();
  control_range(int32_t amin, int32_t amax, int32_t astep, int32_t adef);
  control_range(vector<uint8_t> amin, vector<uint8_t> amax,
                vector<uint8_t> astep, vector<uint8_t> adef);
};

struct guid {
  uint32_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t data4[8];

  string to_string() const;
  operator string() const { return this->to_string(); }
};

// Host driver assigns subdevice and node
// unit and guid are hard-coded in firmware

struct extension_unit {
  int subdevice;
  int unit;
  int node;
  guid id;
   
  string to_string() const;
  operator string() const { return this->to_string(); }
};

enum power_state {
  D0, // full power
  D3  // sleep
};

class uvc_device {
 public:
  typedef std::function<void(const notification&)> error_callback;
 protected:
  error_callback _error_handler;
 public:
  virtual ~uvc_device() = default;
  
  virtual void probe_and_commit(stream_profile prof, frame_callback callback, int buffers = 4) = 0;
  virtual void stream_on(error_callback on_error = [](const notification&){}) = 0;
  virtual void start_callbacks() = 0;
  virtual void stop_callbacks() = 0;
  virtual void close(stream_profile profile) = 0;

  virtual void set_power_state(power_state state) = 0;
  virtual power_state get_power_state() const = 0;

  virtual void init_xu(const extension_unit& xu) = 0;
  virtual bool set_xu(const extension_unit& xu, uint8_t ctrl, const uint8_t* data, int len) = 0;
  virtual bool get_xu(const extension_unit& xu, uint8_t ctrl, uint8_t* data, int len) const = 0;
  virtual control_range get_xu_range(const extension_unit& xu, uint8_t ctrl, int len) const = 0;

  virtual bool get_pu(ru_option opt, int32_t& value) const = 0;
  virtual bool set_pu(ru_option opt, int32_t value) = 0;
  virtual control_range get_pu_range(ru_option opt) const = 0;

  virtual vector<stream_profile> get_profiles() const = 0;

  virtual void lock() const = 0;
  virtual void unlock() const = 0;

  virtual string get_device_location() const = 0;
  virtual usb_spec get_usb_specification() const = 0;
};

class uvc_device_info {
 public:
  string id; // distinguish different pins of one device
  uint16_t vid;
  uint16_t pid;
  uint16_t mi;
  string unique_id;
  string device_path;
  usb_spec conn_spec;
  uint32_t uvc_capabilities;
  bool has_metadata_node;
  string metadata_node_id;
 
 public:
  uvc_device_info();
  ~uvc_device_info() = default;
  bool operator==(const uvc_device_info& b) const;
  bool operator<(const uvc_device_info& b) const;
  string to_string() const;
  operator string() const { return this->to_string(); }
};

class uvc_device_with_retry : public uvc_device {
 private:
   shared_ptr<uvc_device> raw_;
 public:
  explicit uvc_device_with_retry(shared_ptr<uvc_device> raw);
  virtual ~uvc_device_with_retry() = default;
  
  virtual void probe_and_commit(stream_profile prof, frame_callback callback, int buffers = 4);
  virtual void stream_on(error_callback on_error = [](const notification&){});
  virtual void start_callbacks();
  virtual void stop_callbacks();
  virtual void close(stream_profile profile);

  virtual void set_power_state(power_state state);
  virtual power_state get_power_state() const;

  virtual void init_xu(const extension_unit& xu);
  virtual bool set_xu(const extension_unit& xu, uint8_t ctrl, const uint8_t* data, int len);
  virtual bool get_xu(const extension_unit& xu, uint8_t ctrl, uint8_t* data, int len) const;
  virtual control_range get_xu_range(const extension_unit& xu, uint8_t ctrl, int len) const;

  virtual bool get_pu(ru_option opt, int32_t& value) const;
  virtual bool set_pu(ru_option opt, int32_t value);
  virtual control_range get_pu_range(ru_option opt) const;

  virtual vector<stream_profile> get_profiles() const;

  virtual void lock() const;
  virtual void unlock() const;

  virtual string get_device_location() const;
  virtual usb_spec get_usb_specification() const;
};

} // end librealuvc

#endif
