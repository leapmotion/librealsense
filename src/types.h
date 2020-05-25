#ifndef LIBREALUVC_TYPES_H
#define LIBREALUVC_TYPES_H

#include <librealuvc/ru_common.h>
#include <librealuvc/ru_exception.h>
#include <cassert>
#include <cstring>
#include <sstream>

namespace librealuvc {

enum ru_severity {
  RU_SEVERITY_DEBUG,
  RU_SEVERITY_INFO,
  RU_SEVERITY_WARNING,
  RU_SEVERITY_ERROR,
  RU_SEVERITY_FATAL,
  RU_SEVERITY_NONE,
  RU_SEVERITY_COUNT 
};

constexpr ru_severity RS2_LOG_SEVERITY_WARN  = RU_SEVERITY_WARNING;
constexpr ru_severity RS2_LOG_SEVERITY_ERROR = RU_SEVERITY_ERROR;

typedef ru_severity rs2_log_severity;

void log_to_console(ru_severity min_sev);

void log_to_file(ru_severity min_sev, const char* file);

void log_msg(ru_severity sev, const std::string& ss);

#define LOG_WITH_SEVERITY(sev, ...) \
  do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log_msg(sev, ss.str()); \
  } while (false)

#define LOG_DEBUG(...)   LOG_WITH_SEVERITY(librealuvc::RU_SEVERITY_DEBUG,   __VA_ARGS__)
#define LOG_INFO(...)    LOG_WITH_SEVERITY(librealuvc::RU_SEVERITY_INFO,    __VA_ARGS__)
#define LOG_WARNING(...) LOG_WITH_SEVERITY(librealuvc::RU_SEVERITY_WARNING, __VA_ARGS__)
#define LOG_ERROR(...)   LOG_WITH_SEVERITY(librealuvc::RU_SEVERITY_ERROR,   __VA_ARGS__)
#define LOG_FATAL(...)   LOG_WITH_SEVERITY(librealuvc::RU_SEVERITY_FATAL,   __VA_ARGS__)

#pragma pack(push, 1)
template<class T>
class big_endian {
 private:
  T big_endian_val;
 public:
   // FIXME: this assumes that the host is little-endian (that's what you
   //        get for picking up code written by Intel :-)
  inline operator T() const {
    T little_endian_val;
    auto src = reinterpret_cast<const char*>(&big_endian_val);
    auto dst = (reinterpret_cast<char*>(&little_endian_val)) + sizeof(T);
    for (size_t j = 0; j < sizeof(T); ++j) {
      *--dst = *src++;
    }
    return little_endian_val;
  }
};
#pragma pack(pop)

class to_string {
 private:
  std::ostringstream ss_;

 public:
  template<typename T>
  to_string& operator<<(const T& val) { ss_ << val; return *this; }
  
  operator std::string() const { return ss_.str(); }
};

inline void copy(void* dst, const void* src, size_t size) {
  memcpy((dst), (src), (size));
}

#pragma pack(push, 1)
struct uvc_header {
  uint8_t  length; // UVC Metadata total length is max 255 bytes
  uint8_t  info;
  uint32_t timestamp;
  uint8_t  source_clock[6];
};
#pragma pack(pop)

constexpr uint8_t uvc_header_size = sizeof(uvc_header);

} // end librealuvc

#endif
