/*******************************************************************************
 * fcache_profile.hpp
 *
 * Copyright (c) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#ifndef FCACHE_PROFILE
#define FCACHE_PROFILE

#include <cstdio>
#include <string>
#include <unordered_map>

#include "common.hpp"
#include "fcache_count.hpp"

class fcache_profile {

public:
  fcache_profile(const std::string& file_path) {
    log_file_ = fopen(file_path.c_str(), "w");
    fcache_count_set_log_callback(fcache_profile::static_log, this);
    fcache_count_set_register_fd_callback(fcache_profile::static_register_fd, this);
  }

  ~fcache_profile() {
    fclose(log_file_);
  }

  void log_message(const std::string& message) {
    fprintf(log_file_, "# log message: %s\n", message.c_str());
  }

private:
  void log(const file_pageinfo& fpi) {
    std::string file_name = "Unknown";
    auto result = file_names_.find(fpi.fd);
    if (result != file_names_.end()) {
      file_name = result->second;
    }

    fprintf(log_file_, "%s: pages in cache: %zd/%zd (%.1f%%)  [filesize=%.1fK, "
      "pagesize=%.1fK]\n", file_name.c_str(), fpi.nr_pages_cached, fpi.nr_pages,
      fpi.nr_pages == 0 ? 0 : (100.0 * fpi.nr_pages_cached / fpi.nr_pages),
      1.0 * fpi.size / 1024, ((1.0 * fpi.size) / fpi.nr_pages) / 1024);
  }

  void register_fd(const int fd, const std::string& file_name) {
    auto entry = file_names_.find(fd);
    if (entry != file_names_.end()) {
      entry->second = file_name;
    } else {
      file_names_.emplace(fd, file_name);
    }
  }

  static void static_log(void* profile, const file_pageinfo& fpi) {
    static_cast<fcache_profile*>(profile)->log(fpi);
  }

  static void static_register_fd(void* profile, const int fd,
    const char* file_name) {
    static_cast<fcache_profile*>(profile)->register_fd(fd, std::string(file_name));
  }

private:
  FILE* log_file_;
  std::unordered_map<int, std::string> file_names_;

}; // class fcache_profile

#endif // FCACHE_PROFILE

/******************************************************************************/
