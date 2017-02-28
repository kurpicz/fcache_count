/*******************************************************************************
 * fcache_disbale.cpp
 *
 * Some parts are copied (and modified) from https://github.com/feh/nocache
 *
 * Copyright (c) 2011 Julius Plenz <julius@plenz.com>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "fcache_disable.hpp"

int fadv_dontneed(int fd, off_t offset, off_t len, int n) {
  int ret;
  for(int i = 0, ret = 0; i < n && ret == 0; ++i) {
    ret = posix_fadvise(fd, offset, len, POSIX_FADV_DONTNEED);
  }
  return ret;
}

int fadv_noreuse(int fd, off_t offset, off_t len) {
  return posix_fadvise(fd, offset, len, POSIX_FADV_NOREUSE);
}

int valid_fd(int fd) {
  return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

void sync_if_writable(int fd) {
  int r;
  if((r = fcntl(fd, F_GETFL)) == -1) {
    return;
  }
  if((r & O_ACCMODE) != O_RDONLY) {
    fdatasync(fd);
  }
}

int fcntl_dupfd(int fd, int arg) {
  return fcntl(fd, F_DUPFD, arg);
}

/******************************************************************************/
