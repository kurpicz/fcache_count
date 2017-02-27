/*******************************************************************************
 * common.hpp
 *
 * Some parts are copied (and modified) from https://github.com/feh/nocache
 *
 * Copyright (c) 2011 Julius Plenz <julius@plenz.com>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#ifndef COMMON_HEADER
#define COMMON_HEADER

struct byterange {
  size_t pos;
  size_t len;
  struct byterange* next;
}; // struct byterange

struct file_pageinfo {
  int fd;
  off_t size;
  size_t nr_pages;
  size_t nr_pages_cached;
  struct byterange* unmapped;
}; // struct file_pageinfo

#endif // COMMON_HEADER

/******************************************************************************/