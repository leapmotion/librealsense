// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#pragma once
#ifndef LIBREALUVC_BACKEND_H
#define LIBREALUVC_BACKEND_H

#include "../include/librealuvc/realuvc.h"     // Inherit all type definitions in the public API
#include <algorithm>
#include <chrono>
#include <memory>       // For shared_ptr
#include <set>

const uint16_t MAX_RETRIES                = 100;
const uint16_t VID_INTEL_CAMERA           = 0x8086;
const uint8_t  DEFAULT_V4L2_FRAME_BUFFERS = 4;
const uint16_t DELAY_FOR_RETRIES          = 50;

const uint8_t MAX_META_DATA_SIZE          = 0xff; // UVC Metadata total length
                                            // is limited by (UVC Bulk) design to 255 bytes
namespace librealuvc {

// In the original librealsense code, all the backend declarations
// were in librealuvc::platform.  To avoid unnecessary changes to the
// backend, we now declare the outer types, and alias them in platform.

namespace platform {

typedef librealuvc::backend backend;
typedef librealuvc::backend_device_group backend_device_group;
typedef librealuvc::control_range control_range;
typedef librealuvc::frame_object frame_object;
typedef librealuvc::notification notification;
typedef librealuvc::os_time_service os_time_service;
typedef librealuvc::power_state power_state;
typedef librealuvc::time_service time_service;
typedef librealuvc::guid guid;
typedef librealuvc::extension_unit extension_unit;
typedef librealuvc::stream_profile_tuple stream_profile_tuple;
typedef librealuvc::stream_profile stream_profile;
        
std::shared_ptr<backend> create_backend();

class retry_controls_work_around : public uvc_device
{
public:
    explicit retry_controls_work_around(std::shared_ptr<uvc_device> dev)
            : _dev(dev) {}

    void probe_and_commit(stream_profile profile, frame_callback callback, int buffers) override
    {
        _dev->probe_and_commit(profile, callback, buffers);
    }

    void stream_on(std::function<void(const notification& n)> error_handler = [](const notification& n){}) override
    {
        _dev->stream_on(error_handler);
    }

    void start_callbacks() override
    {
        _dev->start_callbacks();
    }

    void stop_callbacks() override
    {
        _dev->stop_callbacks();
    }

    void close(stream_profile profile) override
    {
        _dev->close(profile);
    }

    void set_power_state(power_state state) override
    {
        _dev->set_power_state(state);
    }

    power_state get_power_state() const override
    {
        return _dev->get_power_state();
    }

    void init_xu(const extension_unit& xu) override
    {
        _dev->init_xu(xu);
    }

    bool set_xu(const extension_unit& xu, uint8_t ctrl, const uint8_t* data, int len) override
    {
        for (auto i = 0; i < MAX_RETRIES; ++i)
        {
            if (_dev->set_xu(xu, ctrl, data, len))
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_FOR_RETRIES));
        }
        return false;
    }

    bool get_xu(const extension_unit& xu, uint8_t ctrl, uint8_t* data, int len) const override
    {
        for (auto i = 0; i < MAX_RETRIES; ++i)
        {
            if (_dev->get_xu(xu, ctrl, data, len))
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_FOR_RETRIES));
        }
        return false;
    }

    control_range get_xu_range(const extension_unit& xu, uint8_t ctrl, int len) const override
    {
        return _dev->get_xu_range(xu, ctrl, len);
    }

    bool get_pu(rs2_option opt, int32_t& value) const override
    {
        for (auto i = 0; i < MAX_RETRIES; ++i)
        {
            if (_dev->get_pu(opt, value))
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_FOR_RETRIES));
        }
        return false;
    }

    bool set_pu(rs2_option opt, int32_t value) override
    {
        for (auto i = 0; i < MAX_RETRIES; ++i)
        {
            if (_dev->set_pu(opt, value))
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_FOR_RETRIES));
        }
        return false;
    }

    control_range get_pu_range(rs2_option opt) const override
    {
        return _dev->get_pu_range(opt);
    }

    std::vector<stream_profile> get_profiles() const override
    {
        return _dev->get_profiles();
    }

    std::string get_device_location() const override
    {
        return _dev->get_device_location();
    }

    usb_spec get_usb_specification() const override
    {
        return _dev->get_usb_specification();
    }

    void lock() const override { _dev->lock(); }
    void unlock() const override { _dev->unlock(); }

private:
    std::shared_ptr<uvc_device> _dev;
};

} // end platform

double monotonic_to_realtime(double monotonic);

} // end librealuvc

#endif
