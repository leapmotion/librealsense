// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2021 Ultraleap Ltd. All Rights Reserved.

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace librealuvc {
namespace usb_host {

/*
 * Returns a collection of device name/device descriptor pairs intended to be used with
 * usbhost's usb_device_new() call.
 *
 * At the moment this only works in an app context as it requires a JVM instance. When running a
 * command line application, this function will return an empty vector.
 *
 * Internally it uses JNI calls into the UsbManager class, therefore it requires the application
 * using the library to have USBHOST and CAMERA permissions.
 */
std::vector<std::pair<std::string, int>> enumerate_usb_devices();

}  // namespace usb_host
}  // namespace librealuvc
