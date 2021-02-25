// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2021 Ultraleap Ltd. All Rights Reserved.
#pragma once

#include <jni.h>

#include <cassert>
#include <thread>

namespace librealuvc {
namespace platform {

class java_env final {
public:
  inline JNIEnv* env() {
    assert(std::this_thread::get_id() == m_calling_thread_id);
    return m_env;
  }

  java_env();
  ~java_env();

  java_env(const java_env&) = delete;
  java_env& operator=(const java_env&) = delete;
private:

  JavaVM* m_jvm = nullptr;
  JNIEnv* m_env = nullptr;

  bool m_detach = false;
  std::thread::id m_calling_thread_id = std::this_thread::get_id();
};

}  // namespace platform
}  // namespace librealuvc
