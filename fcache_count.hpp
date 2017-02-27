/*******************************************************************************
 * fcache_count.hpp
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

#include "common.hpp"

using fcache_count_log_callback_type = void (*)(void* profile, const file_pageinfo& message);
using fcache_count_register_fd_callback_type = void (*)(void* profile, int fd, const char* file_name);
void fcache_count_set_log_callback(fcache_count_log_callback_type call, void* profile);
void fcache_count_set_register_fd_callback(fcache_count_register_fd_callback_type call, void* profile);

void init_mutexes();
void free_unclaimed_pages(int fd);
int insert_into_br_list(struct file_pageinfo *pi, struct byterange** brtail,
	size_t pos, size_t len);
file_pageinfo* fd_get_pageinfo(int fd, struct file_pageinfo* pi);
void free_br_list(struct byterange** br);
void store_pageinfo(int fd);
void free_unclaimed_pages(int fd);

#endif // FCACHE_COUNT_HEADER

/******************************************************************************/