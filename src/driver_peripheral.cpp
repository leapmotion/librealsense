// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 Leap Motion Corporation. All Rights Reserved.

#include "drivers.h"
#include <librealuvc/ru_uvc.h>
#include <librealuvc/ru_videocapture.h>
#include "leap_xu.h"
#include <cstdio>

#if 1
#define D(...) { printf("DEBUG[%s,%d] ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#define D(...) { }
#endif

namespace librealuvc {

using std::shared_ptr;

static int32_t saturate(double val, int32_t min, int32_t max) {
  int32_t ival = (int32_t)val;
  if (ival < min) return min;
  if (ival > max) return max;
  return ival;
}

static int32_t flag(double val) {
  return ((val == 0.0) ? 0 : 1);
}

class PropertyDriverPeripheral : public IPropertyDriver {
 private:
  shared_ptr<uvc_device> dev_;
  extension_unit leap_xu_;
  double hdr_;
  double leds_;
 public:
  PropertyDriverPeripheral(const shared_ptr<uvc_device>& dev) :
    dev_(dev),
    hdr_(0),
    leds_(1) {
    leap_xu_.subdevice = 0;
    leap_xu_.unit = 1;
    leap_xu_.node = 4;
    leap_xu_.id = guid LEAP_XU_GUID;
    try {
      dev_->init_xu(leap_xu_);
    } catch (std::exception& e) {
      printf("EXCEPTION: init_xu: %s\n", e.what());
    }
  }
  
  bool is_stereo_camera() override {
    return true;
  }

  DevFrameFixup get_frame_fixup() override {
    return FIXUP_GRAY8_PIX_L_PIX_R;
  }

  shared_ptr<OpaqueCalibration> get_opaque_calibration() override {
    const size_t kCalibrationDataSize = 156;
    vector<uint8_t> data(kCalibrationDataSize);
    // Use RU_OPTION_SHARPNESS  as property-knocker byte address
    //     RU_OPTION_SATURATION as property-knocker byte value
    const int32_t kMemCalibAddr = 100;
    for (int32_t offset = 0; offset < (int32_t)kCalibrationDataSize; ++offset) {
      dev_->set_pu(RU_OPTION_SHARPNESS, kMemCalibAddr+offset);
      int32_t val = 0;
      dev_->get_pu(RU_OPTION_SATURATION, val);
      data[offset] = (uint8_t)val;
    }
    return std::make_shared<OpaqueCalibration>("LeapStereoCalibration", 1, 0, 0, data);
  }

  HandlerResult get_prop(int prop_id, double* val) override {
    bool ok = true;
    int32_t ival = 0;
    switch (prop_id) {
      case cv::CAP_PROP_EXPOSURE:
        ok = dev_->get_pu(RU_OPTION_ZOOM_ABSOLUTE, ival);
        *val = (double)ival;
        break;
      case cv::CAP_PROP_GAIN:
        ok = dev_->get_pu(RU_OPTION_GAIN, ival);
        *val = (double)ival;
        break;
      case cv::CAP_PROP_GAMMA:
        ok = dev_->get_pu(RU_OPTION_GAMMA, ival);
        *val = (double)ival;
        break;
      case CAP_PROP_LEAP_HDR:
        *val = hdr_;
        break;
      case CAP_PROP_LEAP_LEDS:
        *val = leds_;
        break;
      default:
        return kHandlerNotDone;
    }
    return (ok ? kHandlerTrue : kHandlerFalse);
  }
  
  HandlerResult get_prop_range(int prop_id, double* min_val, double* max_val) override {
    bool ok = true;
    *min_val = 0;
    *max_val = 0;
    switch (prop_id) {
      case cv::CAP_PROP_EXPOSURE:
        *min_val = 10;
        *max_val = 300;
        break;
      case cv::CAP_PROP_GAIN:
        *min_val = 16;
        *max_val = 63;
        break;
      case cv::CAP_PROP_GAMMA:
        *max_val = 1;
        break;
      case CAP_PROP_LEAP_HDR:
        *max_val = 1;
        break;
      case CAP_PROP_LEAP_LEDS:
        *max_val = 1;
        break;
      default:
        return kHandlerNotDone;
    }
    return (ok ? kHandlerTrue : kHandlerFalse);
  }
  
  HandlerResult set_prop(int prop_id, double val) override {
    bool ok = true;
    switch (prop_id) {
      case cv::CAP_PROP_EXPOSURE:
        ok = dev_->set_pu(RU_OPTION_ZOOM_ABSOLUTE, saturate(val, 10, 0xffff));
        break;
      case cv::CAP_PROP_GAIN:
        ok = dev_->set_pu(RU_OPTION_GAIN, saturate(val, 16, 63));
        break;
      case cv::CAP_PROP_GAMMA:
        ok = dev_->set_pu(RU_OPTION_GAMMA, flag(val));
        break;
      case CAP_PROP_LEAP_HDR:
        hdr_ = val;
        ok = dev_->set_pu(RU_OPTION_CONTRAST, 0x0 | (flag(val)<<6));
        break;
      case CAP_PROP_LEAP_LEDS:
        ok &= dev_->set_pu(RU_OPTION_CONTRAST, 0x2 | (flag(val)<<6));
        ok &= dev_->set_pu(RU_OPTION_CONTRAST, 0x3 | (flag(val)<<6));
        ok &= dev_->set_pu(RU_OPTION_CONTRAST, 0x4 | (flag(val)<<6));
        if (ok) leds_ = val;
        break;
      default:
        return kHandlerNotDone;
    }
    return (ok ? kHandlerTrue : kHandlerFalse);
  }
};

#define VENDOR_PERIPHERAL  0xf182
#define PRODUCT_PERIPHERAL 0x0003

void import_driver_peripheral() {
  register_property_driver(
    VENDOR_PERIPHERAL,
    PRODUCT_PERIPHERAL,
    [](const shared_ptr<uvc_device>& realuvc)->shared_ptr<IPropertyDriver> {
      return std::make_shared<PropertyDriverPeripheral>(realuvc);
    }
  );
}

} // end librealuvc
