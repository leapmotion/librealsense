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

static const int CY_FX_UVC_PU_GAIN_SELECTOR_GAIN  = 0x4000;
static const int CY_FX_UVC_PU_GAIN_SELECTOR_FOCUS = 0x8000;
static const int CY_FX_UVC_PU_GAIN_SELECTOR_WHITE = 0xc000;

class PropertyDriverRigel : public IPropertyDriver {
 private:
  shared_ptr<uvc_device> dev_;
  extension_unit leap_xu_;
  double leds_;
 public:
  PropertyDriverRigel(const shared_ptr<uvc_device>& dev) :
    dev_(dev),
    leds_(0) {
    D("PropertyDriverRigel::ctor() ...");
    leap_xu_.subdevice = 0;
    leap_xu_.unit = 224;
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
    return FIXUP_GRAY8_ROW_L_ROW_R;
  }

  shared_ptr<OpaqueCalibration> get_opaque_calibration() override {
    const size_t kCalibrationDataSize = 156;
    vector<uint8_t> data(kCalibrationDataSize);
    bool ok = dev_->get_xu(leap_xu_, LEAP_XU_CALIBRATION_DATA, &data[0], (int)data.size());
    if (!ok) return nullptr;
    return std::make_shared<OpaqueCalibration>("LeapStereoCalibration", 1, 0, 0, data);
  }

  HandlerResult get_prop(int prop_id, double* val) override {
    bool ok = true;
    int32_t ival = 0;
    switch (prop_id) {
      case cv::CAP_PROP_BRIGHTNESS:
        *val = 0.0;
        break;
      case cv::CAP_PROP_EXPOSURE: {
        uint16_t tmp = 0;
        ok = dev_->get_xu(leap_xu_, LEAP_XU_EXPOSURE_CONTROL, (uint8_t*)&tmp, sizeof(tmp));
        *val = (double)tmp;
        break;
      }
      case cv::CAP_PROP_GAIN: {
        // never use values 1-15
        ok = dev_->get_pu(RU_OPTION_GAIN, ival);
        ival = ((ival < 16) ? 0 : ival-15);
        *val = (double)ival;
        break;
      }
      case cv::CAP_PROP_GAMMA:
        *val = 0.0;
        break;
      case CAP_PROP_LEAP_HDR:
        *val = 0.0;
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
      case cv::CAP_PROP_BRIGHTNESS:
        // Rigel has a brightness control, but it doesn't do anything
        ok = false;
        break;
      case cv::CAP_PROP_EXPOSURE:
        *max_val = 4000;
        break;
      case cv::CAP_PROP_GAIN: {
        // skip the range 1-15 which has non-monotonic effect
        *min_val = 0;
        *max_val = 79-15;
        break;
      }
      case cv::CAP_PROP_GAMMA:
        ok = false;
        break;
      case CAP_PROP_LEAP_HDR:
        ok = false;
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
      case cv::CAP_PROP_BRIGHTNESS:
        ok = ((val == 0.0) ? true : false);
        break;
      case cv::CAP_PROP_EXPOSURE: {
        // Exposure goes through set_xu
        uint16_t tmp = (uint16_t)saturate(val, 10, 0xffff);
        ok = dev_->set_xu(leap_xu_, LEAP_XU_EXPOSURE_CONTROL, (uint8_t*)&tmp, sizeof(tmp));
        break;
      }
      case cv::CAP_PROP_GAIN: {
        if (val >= 1.0) val += 15.0;
        ok = dev_->set_pu(RU_OPTION_GAIN, saturate(val, 0, 79));
        break;
      }
      case cv::CAP_PROP_GAMMA:
        // No hardware gamma
        ok = ((val == 0.0) ? true : false);
        break;
      case CAP_PROP_LEAP_HDR:
        // No hardware HDR
        ok = ((val == 0.0) ? true : false);
        break;
      case CAP_PROP_LEAP_LEDS: {
        // LED control goes through set_xu
        uint8_t tmpA = flag(val);
        ok = dev_->set_xu(leap_xu_, LEAP_XU_STROBE_CONTROL, (uint8_t*)&tmpA, sizeof(tmpA));
        uint32_t tmpB = flag(val);
        ok &= dev_->set_xu(leap_xu_, LEAP_XU_ESC_LED_CHARGE, (uint8_t*)&tmpB, sizeof(tmpB));
        if (ok) leds_ = val;
        break;
      }
      default:
        return kHandlerNotDone;
    }
    return (ok ? kHandlerTrue : kHandlerFalse);
  }
};

#define VENDOR_RIGEL  0x2936
#define PRODUCT_RIGEL 0x1202

void import_driver_rigel() {
  register_property_driver(
    VENDOR_RIGEL,
    PRODUCT_RIGEL,
    [](const shared_ptr<uvc_device>& realuvc)->shared_ptr<IPropertyDriver> {
      return std::make_shared<PropertyDriverRigel>(realuvc);
    }
  );
}

} // end librealuvc
