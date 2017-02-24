/*******************************************************************************
 * fcache_count.cpp
 *
 * Some parts are copied (and modified) from https://github.com/feh/nocache
 *
 * Copyright (c) 2011 Julius Plenz <julius@plenz.com>
 * Copyright (c) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#ifndef FCACHE_COUNT_HEADER
#define FCACHE_COUNT_HEADER

struct byterange {
  size_t pos, len;
  struct byterange *next;
}; // struct byterange

struct file_pageinfo {
  int fd;
  off_t size;
  size_t nr_pages;
  size_t nr_pages_cached;
  struct byterange *unmapped;
}; // struct file_pageinfo

using fcache_count_callback_type = void (*)(void* profile, const char* message);
void fcache_count_set_callback(fcache_count_callback_type call, void* profile);

void init_mutexes();
void free_unclaimed_pages(int fd);
void fcache_count_set_callback(fcache_count_callback_type call, void* profile);
int insert_into_br_list(struct file_pageinfo *pi, struct byterange **brtail,
	size_t pos, size_t len);
file_pageinfo* fd_get_pageinfo(int fd, struct file_pageinfo *pi);
void free_br_list(struct byterange **br);
void store_pageinfo(int fd);
void free_unclaimed_pages(int fd);

#endif // FCACHE_COUNT_HEADER

/******************************************************************************/