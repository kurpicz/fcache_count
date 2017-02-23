#ifndef FCACHE_PROFILE
#define FCACHE_PROFILE

#include <cstdio>
#include <string>

#include "fcache_count.hpp"

class fcache_profile {

public:
  fcache_profile(const std::string& file_path) {
    log_file_ = fopen(file_path.c_str(), "w");
    fcache_count_set_callback(fcache_profile::static_log, this);
  }

  ~fcache_profile() {
    fclose(log_file_);
  }

private:
  void log(const char* message) {
    fprintf(log_file_, "%s\n", message);
  }

  static void static_log(void* profile, const char* message) {
    static_cast<fcache_profile*>(profile)->log(message);
  }

private:
  FILE* log_file_;

}; // class fcache_profile

#endif // FCACHE_PROFILE