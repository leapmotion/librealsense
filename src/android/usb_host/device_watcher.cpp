// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 Intel Corporation. All Rights Reserved.

#ifdef RS2_USE_ANDROID_BACKEND
#include "device_watcher.h"
#include <vector>
#include <jni.h>
#include "usb_device.h"
#include "../../backend.h"

using namespace librealuvc;
using namespace librealuvc::usb_host;

static std::vector<std::shared_ptr<device>> _devices;
static librealuvc::device_changed_callback _callback = nullptr;

std::vector<std::shared_ptr<device>> android_device_watcher::get_device_list()
{
    return _devices;
}

void android_device_watcher::start(librealuvc::device_changed_callback callback)
{
    _callback = callback;
}

void android_device_watcher::stop()
{
    _callback = nullptr;
}

std::vector<uvc_device_info> android_device_watcher::query_uvc_devices() {
    std::vector<uvc_device_info> devices;
    for (auto dev : _devices) {
        for (int ia = 0; ia < dev->get_interfaces_associations_count(); ia++) {
            auto iad = dev->get_interface_association(ia);
            if (iad.get_descriptor()->bFunctionClass == USB_CLASS_VIDEO) {
                uvc_device_info device_info;
                device_info.vid = dev->get_vid();
                device_info.pid = dev->get_pid();
                device_info.mi = iad.get_mi();
                device_info.unique_id = dev->get_file_descriptor();
                device_info.device_path = dev->get_name();
                device_info.conn_spec = usb_spec(dev->get_conn_spec());
                LOG_INFO("Found UVC Device vid: " << std::string(device_info).c_str());
                devices.push_back(device_info);
            }
        }
    }
    return devices;
}

void android_device_watcher::addUsbDevice(const std::string& deviceName, int fileDescriptor) {
    auto find_device = [&deviceName](const std::shared_ptr<device>& d) {
        if (d->get_name() == deviceName) return true;
        return false;
    };

    if (std::find_if(_devices.cbegin(), _devices.cend(), find_device) != _devices.cend()) {
      LOG_INFO(deviceName << " already in cache. Not adding again");
      return;
    }

    backend_device_group prev;
    prev.uvc_devices = android_device_watcher::query_uvc_devices();
    LOG_DEBUG("AddUsbDevice, previous device count: " << prev.uvc_devices.size());
    LOG_DEBUG("AddUsbDevice, adding device: " << deviceName << ", descriptor: " << fileDescriptor);

    auto handle = usb_device_new(deviceName.c_str(), fileDescriptor);

    if (handle != NULL) {
        auto d = std::make_shared<device>(handle);
        _devices.push_back(d);
    }

    backend_device_group curr;
    curr.uvc_devices = android_device_watcher::query_uvc_devices();

    if(_callback)
        _callback(prev, curr);

    LOG_DEBUG("AddUsbDevice, current device count: " << curr.uvc_devices.size());
}

void android_device_watcher::removeUsbDevice(int fileDescriptor) {
    backend_device_group prev;
    prev.uvc_devices = android_device_watcher::query_uvc_devices();
    LOG_DEBUG("RemoveUsbDevice, previous device count: " << prev.uvc_devices.size());

    _devices.erase(std::remove_if(_devices.begin(), _devices.end(), [fileDescriptor](std::shared_ptr<device> d)
    {
        if(fileDescriptor == d->get_file_descriptor()){
            d->release();
            LOG_DEBUG("RemoveUsbDevice, removing device: " << d->get_name().c_str() << ", descriptor: " << fileDescriptor);
            return true;
        }
        return false;
    }), _devices.end());

    backend_device_group curr;
    curr.uvc_devices = android_device_watcher::query_uvc_devices();

    if(_callback)
        _callback(prev, curr);
    LOG_DEBUG("RemoveUsbDevice, current device count: " << curr.uvc_devices.size());
}

#endif
