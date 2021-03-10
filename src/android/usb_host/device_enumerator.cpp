// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2021 Ultraleap Ltd. All Rights Reserved.
#ifdef RS2_USE_ANDROID_BACKEND

#include "device_enumerator.h"

#include "../jni/java_env.h"

#include "../../types.h"

#include <stdexcept>

namespace librealuvc {
namespace usb_host {

using device_vector = std::vector<std::pair<std::string, int>>;
device_vector enumerate_usb_devices() {
  device_vector result{};

  try {
    auto jenv = librealuvc::platform::java_env();
    JNIEnv* env = jenv.env();

    // Grab the UsbManager instance and class
    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jmethodID activityThreadMethodId = env->GetStaticMethodID(activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject activityThreadObject = env->CallStaticObjectMethod(activityThreadClass, activityThreadMethodId);

    jmethodID getAppMethodId = env->GetMethodID(activityThreadClass, "getApplication", "()Landroid/app/Application;");
    jobject appObject = env->CallObjectMethod(activityThreadObject, getAppMethodId);

    jclass contextClass = env->FindClass("android/content/Context");
    jfieldID usbServiceField = env->GetStaticFieldID(contextClass, "USB_SERVICE", "Ljava/lang/String;");
    jobject usbServiceString = env->GetStaticObjectField(contextClass, usbServiceField);

    jmethodID usbManagerMethodID = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject usbManagerObject = env->CallObjectMethod(appObject, usbManagerMethodID, usbServiceString);

    jclass usbManagerClass = env->FindClass("android/hardware/usb/UsbManager");

    // Get a list of USB devices (HashMap<String, UsbDevice>)
    jmethodID deviceListMethodID = env->GetMethodID(usbManagerClass, "getDeviceList", "()Ljava/util/HashMap;");
    jobject deviceList = env->CallObjectMethod(usbManagerObject, deviceListMethodID);

    // Set up the PendingIntent which will listen for permission requests. This needs to remain in
    // scope for the duration of device enumeration.
    jclass intentClass = env->FindClass("android/content/Intent");
    jmethodID intentCtorId = env->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;)V");
    jstring jACTION_USB_PERMISSION = env->NewStringUTF("com.librealuvc.USBHOST_PERMISSION");
    jobject intentObject = env->NewObject(intentClass, intentCtorId, jACTION_USB_PERMISSION);
    jclass pendingIntentClass = env->FindClass("android/app/PendingIntent");
    jmethodID getBroadcastMethodId = env->GetStaticMethodID(pendingIntentClass, "getBroadcast", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;");
    jobject permissionIntentObject = env->CallStaticObjectMethod(pendingIntentClass, getBroadcastMethodId, appObject, 0, intentObject, 0);

    // Get the objects needed to iterate over the hash map
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapValuesMethodID = env->GetMethodID(hashMapClass, "values", "()Ljava/util/Collection;");
    jobject deviceListCollection = env->CallObjectMethod(deviceList, hashMapValuesMethodID);

    jclass collectionClass = env->FindClass("java/util/Collection");
    jmethodID collectionIteratorMethodID = env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;");
    jobject collectionIteratorObject = env->CallObjectMethod(deviceListCollection, collectionIteratorMethodID);

    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass usbDeviceClass = env->FindClass("android/hardware/usb/UsbDevice");
    jmethodID iteratorHasNextMethodID = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID iteratorNextMethodID = env->GetMethodID(iteratorClass,  "next", "()Ljava/lang/Object;");

    jmethodID usbDeviceNameMethodID = env->GetMethodID(usbDeviceClass, "getDeviceName", "()Ljava/lang/String;");
    jmethodID usbManagerOpenDeviceMethodID = env->GetMethodID(usbManagerClass, "openDevice", "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/UsbDeviceConnection;");
    jmethodID usbManagerHasPermissionMethodID = env->GetMethodID(usbManagerClass, "hasPermission", "(Landroid/hardware/usb/UsbDevice;)Z");
    jclass usbDeviceConnectionClass = env->FindClass("android/hardware/usb/UsbDeviceConnection");
    jmethodID usbDeviceConnectionGetFileDescMethodID = env->GetMethodID(usbDeviceConnectionClass, "getFileDescriptor", "()I");
    jmethodID requestPermissionMethodId = env->GetMethodID(usbManagerClass, "requestPermission", "(Landroid/hardware/usb/UsbDevice;Landroid/app/PendingIntent;)V");

    // Iterate over the device list
    while((bool)env->CallBooleanMethod(collectionIteratorObject, iteratorHasNextMethodID)) {
      jobject device = env->CallObjectMethod(collectionIteratorObject, iteratorNextMethodID);
      jstring jDeviceName = (jstring)env->CallObjectMethod(device, usbDeviceNameMethodID);

      std::string deviceName = env->GetStringUTFChars(jDeviceName, 0);
      /*
       * For proof of concept, asking for permissions synchronously via the JNI API will have to do.
       *
       * This is as far from an ideal solution as you can get, as with the way it is called brings
       * the potential for us to spin infinitely if permissions are denied.
       *
       * What we really need to do, is the following:
       *   - Have a small AAR library built by this project. This will have a device watcher Java
       *   class. When instantiated it will enumerate USB devices and ask for permissions as it
       *   needs.
       *   - Export a JNI interface from the device_watcher C++ class, which will allow us to add
       *   and remove devices.
       *   - Bundle librealuvc into the AAR library, and call its JNI interface from the Java class
       *   when permissions are granted.
       */
      bool allowed = (bool)env->CallBooleanMethod(usbManagerObject, usbManagerHasPermissionMethodID, device);
      if (!allowed) {
        LOG_WARNING("No permissions to use " << deviceName << ". Requesting...");
        env->CallVoidMethod(usbManagerObject, requestPermissionMethodId, device, permissionIntentObject);

        do {
          LOG_WARNING("No permissions to use " << deviceName << ". Waiting...");
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          allowed = (bool)env->CallBooleanMethod(usbManagerObject, usbManagerHasPermissionMethodID, device);
        } while (!allowed);
      }
      LOG_INFO("Got permissions for device " << deviceName);

      jobject deviceConnection = env->CallObjectMethod(usbManagerObject, usbManagerOpenDeviceMethodID, device);

      int deviceFileDescriptor = env->CallIntMethod(deviceConnection, usbDeviceConnectionGetFileDescMethodID);

      LOG_DEBUG(deviceName << " file descriptor " << deviceFileDescriptor);

      result.emplace_back(deviceName, deviceFileDescriptor);
    }

    env->DeleteLocalRef(jACTION_USB_PERMISSION);

    return result;
  } catch (const std::runtime_error& e) {
    LOG_WARNING("No Java VM available: %s", e.what());
    LOG_INFO("Unable to enumerate usbhost devices");
    return result;
  }
}

}  // namespace usb_host
}  // namespace librealuvc

#endif  // RS2_USE_ANDROID_BACKEND
