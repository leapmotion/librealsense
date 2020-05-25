#include <cstdio>
#include <mutex>
#include "types.h"

#ifdef RS2_USE_ANDROID_BACKEND
#include <android/log.h>
#define  LOG_TAG    "librealuvc"

#define  ANDROID_LOG_FATAL(...) __android_log_print(ANDROID_LOG_FATAL,LOG_TAG,__VA_ARGS__)
#define  ANDROID_LOG_ERROR(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  ANDROID_LOG_WARNING(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  ANDROID_LOG_DEBUG(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  ANDROID_LOG_INFO(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif


namespace librealuvc {
  
namespace { // anon

class single_logger {
 public:
  std::mutex mutex_;
  ru_severity sev_console_;
  ru_severity sev_file_;
  FILE* log_;
 
 public:
  single_logger() :
    mutex_(),
    sev_console_(RU_SEVERITY_ERROR),
    sev_file_(RU_SEVERITY_NONE),
    log_(nullptr) {
  }
  
  void log_to_file(ru_severity min_sev, const char* file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_) fclose(log_);
    log_ = fopen(file_path, "w");
    if (!log_) {
      fprintf(stderr, "ERROR: can't write to logfile \"%s\"\n", file_path);
      exit(1);
    }
    sev_file_ = min_sev;
  }
  
  static const char* sev2str(ru_severity sev) {
    switch (sev) {
      case RU_SEVERITY_DEBUG:   return "DEBUG";
      case RU_SEVERITY_INFO:    return "INFO";
      case RU_SEVERITY_WARNING: return "WARNING";
      case RU_SEVERITY_ERROR:   return "ERROR";
      case RU_SEVERITY_FATAL:   return "FATAL";
      default: return "UNKNOWN";
    }
  }
  
  void log_msg(ru_severity sev, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    bool want_console = (sev >= sev_console_);
    bool want_file = ((sev >= sev_file_) && log_);
    if (!want_console && !want_file) return;
    auto sev_str = sev2str(sev);
    if (want_console) {
      printf("%s: librealuvc: ", sev_str);
      fwrite(msg.data(), 1, msg.length(), stdout);
      fputc('\n', stdout);
      fflush(stdout);
    }
    if (want_file) {
      fprintf(log_, "%s: librealuvc: ", sev_str);
      fwrite(msg.data(), 1, msg.length(), log_);
      fputc('\n', stdout);
      fflush(log_);
    }
  }
};

single_logger* get_single_logger() {
  static single_logger single;
  return &single;
}

} // end anon

void log_to_console(ru_severity min_sev) {
  get_single_logger()->sev_console_ = min_sev;
}

void log_to_file(ru_severity min_sev, const char* file_path) {
  get_single_logger()->log_to_file(min_sev, file_path);
}

#ifdef RS2_USE_ANDROID_BACKEND
void android_log_msg(ru_severity sev, const std::string& msg) {
      switch (sev) {
        case RU_SEVERITY_DEBUG:   ANDROID_LOG_WARNING("%s",msg.c_str()); break;
        case RU_SEVERITY_INFO:    ANDROID_LOG_INFO("%s",msg.c_str()); break;
        case RU_SEVERITY_WARNING: ANDROID_LOG_WARNING("%s",msg.c_str()); break;
        case RU_SEVERITY_ERROR:   ANDROID_LOG_ERROR("%s",msg.c_str()); break;
        case RU_SEVERITY_FATAL:   ANDROID_LOG_FATAL("%s",msg.c_str()); break;
        default:;
      }
    }
#endif

void log_msg(ru_severity sev, const std::string& msg) {
#ifdef RS2_USE_ANDROID_BACKEND
  android_log_msg(sev,msg);
#else
  get_single_logger()->log_msg(sev, msg);
#endif
}
}
