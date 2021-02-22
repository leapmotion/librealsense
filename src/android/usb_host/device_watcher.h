/* License: Apache 2.0. See LICENSE file in root directory. */
/* Copyright(c) 2019 Intel Corporation. All Rights Reserved. */
#pragma once

#include "usbhost.h"
#include "usb_device.h"
#include "../../types.h"
#include <memory>
#include <string>

#include <librealuvc/realuvc.h>

namespace librealuvc
{
    namespace usb_host
    {
        class android_device_watcher : public device_watcher
        {
        public:
            virtual void start(device_changed_callback callback) override;
            virtual void stop() override;
            static std::vector<std::shared_ptr<device>> get_device_list();
            static std::vector<uvc_device_info> query_uvc_devices();
            static void addUsbDevice(const std::string& deviceName_, int fileDescriptor);
            static void removeUsbDevice(int fileDescriptor);
        };
    }
}
