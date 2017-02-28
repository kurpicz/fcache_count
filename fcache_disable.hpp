/*******************************************************************************
 * fcache_disbale.hpp
 *
 * Some parts are copied (and modified) from https://github.com/feh/nocache
 *
 * Copyright (c) 2011 Julius Plenz <julius@plenz.com>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#ifndef FCACHE_DISABLE_HEADER
#define FCACHE_DISABLE_HEADER

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int fadv_dontneed(int fd, off_t offset, off_t len, int n);
int fadv_noreuse(int fd, off_t offset, off_t len);
int valid_fd(int fd);
void sync_if_writable(int fd);
int fcntl_dupfd(int fd, int arg);

#endif // FCACHE_DISABLE_HEADER

/******************************************************************************/