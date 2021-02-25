// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2021 Ultraleap Ltd. All Rights Reserved.
#ifdef RS2_USE_ANDROID_BACKEND

#include "java_env.h"

#include <librealuvc/realuvc.h>
#include "../../types.h"

#include <cassert>
#include <stdexcept>
#include <string>

namespace librealuvc {

static JavaVM* jvm_p = nullptr;
void set_java_vm(JavaVM* jvm) { jvm_p = jvm; }

namespace platform {

java_env::java_env() :
  m_jvm(jvm_p) {

  if (m_jvm == nullptr) {
    throw std::runtime_error("Invalid Java VM!");
  }

  auto res = m_jvm->GetEnv(reinterpret_cast<void**>(&m_env), JNI_VERSION_1_6);

  // The JNIEnv pointer is only valid on the current JVM thread. If this is not the case GetEnv
  // sets it to NULL and we need to call AttachCurrentThread().
  // See https://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html and the
  // section "Attaching to the VM"
  if (res == JNI_EDETACHED) {
    auto attach = m_jvm->AttachCurrentThread(&m_env, nullptr);

    if (attach != JNI_OK) {
      throw std::runtime_error("Unable to attach to JVM thread");
      m_detach = true;
    }
  } else if (res != JNI_OK) {
    throw std::runtime_error("Unexpected JNI GetEnv result");
  }

  if (m_env == nullptr) {
    throw std::runtime_error("Invalid JNI environment");
  }
}

java_env::~java_env() {
  assert(std::this_thread::get_id() == m_calling_thread_id);
  if (m_jvm != nullptr && m_detach) {
    m_jvm->DetachCurrentThread();
  }

  m_env = nullptr;
  m_jvm = nullptr;
}

}  // namespace platform
}  // namespace librealuvc

#endif  // RS2_USE_ANDROID_BACKEND
