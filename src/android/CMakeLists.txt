# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.

target_sources(${LRS_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usbhost.c"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usbhost.h"

        "${CMAKE_CURRENT_LIST_DIR}/android-uvc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/android-uvc.h"
        "${CMAKE_CURRENT_LIST_DIR}/android-hid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/android-hid.h"
        "${CMAKE_CURRENT_LIST_DIR}/android-backend.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/android-backend.h"

        "${CMAKE_CURRENT_LIST_DIR}/usb_host/android_uvc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/android_uvc.h"

        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_endpoint.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_interface.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_interface_association.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_configuration.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_pipe.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_pipe.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_device.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/usb_device.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/usb_host/device_watcher.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/device_watcher.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/usb_host/device_enumerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/usb_host/device_enumerator.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/jni/java_env.h"
        "${CMAKE_CURRENT_LIST_DIR}/jni/java_env.cpp"
)

message(STATUS "Prepare librealuvc for Android OS (${ANDROID_NDK_ABI_NAME})")
