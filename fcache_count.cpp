/*******************************************************************************
 * fcache_count.cpp
 *
 * Some parts are copied (and modified) from https://github.com/feh/nocache
 *
 * Copyright (C) 2011 Julius Plenz <julius@plenz.com>
 * Copyright (C) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "fcache_count.hpp"

static fcache_count_callback_type callback = nullptr;
static void* callback_profile = nullptr;

static int max_fds;
static struct file_pageinfo *fds;
static size_t PAGESIZE;

using open_create_type = int (*)(const char *, int, mode_t);
using open_at_type     = int (*)(int, const char *, int, mode_t);
using dup_type         = int (*)(int);
using dup2_type        = int (*)(int, int);
using close_type       = int (*)(int);
using fopen_type       = FILE *(*)(const char *, const char *);
using fclose_type      = int (*)(FILE *);

static open_create_type original_open = nullptr;
static open_create_type original_open64 = nullptr;
static open_create_type original_creat = nullptr;
static open_create_type original_creat64 = nullptr;
static open_at_type original_openat = nullptr;
static open_at_type original_openat64 = nullptr;
static dup_type original_dup = nullptr;
static dup2_type original_dup2 = nullptr;
static close_type original_close = nullptr;
static fopen_type original_fopen = nullptr;
static fopen_type original_fopen64 = nullptr;
static fclose_type original_fclose = nullptr;

static pthread_mutex_t* fds_lock;
static pthread_mutex_t fds_iter_lock;

static __attribute__ ((constructor)) void init() {

  PAGESIZE = getpagesize();

  struct rlimit rlim;
  getrlimit(RLIMIT_NOFILE, &rlim);
  max_fds = rlim.rlim_max;
  fds = static_cast<file_pageinfo*>(malloc(max_fds * sizeof(*fds)));

  init_mutexes();

  original_open = (open_create_type)(dlsym(RTLD_NEXT, "open"));
  if (!original_open) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_open64 = (open_create_type)(dlsym(RTLD_NEXT, "open64"));
  if (!original_open64) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_creat = (open_create_type)(dlsym(RTLD_NEXT, "creat"));
  if (!original_creat) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_creat64 = (open_create_type)(dlsym(RTLD_NEXT, "creat64"));
  if (!original_creat64) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_openat = (open_at_type)(dlsym(RTLD_NEXT, "openat"));
  if (!original_openat) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_openat64 = (open_at_type)(dlsym(RTLD_NEXT, "openat64"));
  if (!original_openat64) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_dup = (dup_type)(dlsym(RTLD_NEXT, "dup"));
  if (!original_dup) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_dup2 = (dup2_type)(dlsym(RTLD_NEXT, "dup2"));
  if (!original_dup2) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_close = (close_type)(dlsym(RTLD_NEXT, "close"));
  if (!original_close) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_fopen = (fopen_type)(dlsym(RTLD_NEXT, "fopen"));
  if (!original_fopen) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_fopen64 = (fopen_type)(dlsym(RTLD_NEXT, "fopen64"));
  if (!original_fopen64) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }

  original_fclose = (fclose_type)(dlsym(RTLD_NEXT, "fclose"));
  if (!original_fclose) {
    fprintf(stderr, "dlerror %s\n", dlerror());
    std::exit(EXIT_FAILURE);
  }
}

void init_mutexes() {
    int i;
    pthread_mutex_init(&fds_iter_lock, nullptr);
    pthread_mutex_lock(&fds_iter_lock);
    fds_lock = static_cast<pthread_mutex_t*>(malloc(max_fds * sizeof(*fds_lock)));
    for(i = 0; i < max_fds; i++) {
        pthread_mutex_init(&fds_lock[i], nullptr);
    }
    pthread_mutex_unlock(&fds_iter_lock);
    /* make sure to re-initialize mutex if forked */
    pthread_atfork(nullptr, nullptr, init_mutexes);
}

int open(const char *path_name, int flags, mode_t mode) {
  if(!original_open) {
    original_open = (open_create_type)(dlsym(RTLD_NEXT, "open"));
  }
  int fd = original_open(path_name, flags, mode);
  store_pageinfo(fd);
  return fd;
}

int open64(const char *path_name, int flags, mode_t mode) {
  if (!original_open64) {
    original_open64 = (open_create_type)(dlsym(RTLD_NEXT, "open64"));
  }
  int fd = original_open64(path_name, flags, mode);
  store_pageinfo(fd);
  return fd;
}

int creat(const char *path_name, int flags, mode_t mode) {
  if (!original_creat) {
    original_creat = (open_create_type)(dlsym(RTLD_NEXT, "creat"));
  }
  int fd = original_creat(path_name, flags, mode);
  return fd;
}

int creat64(const char *path_name, int flags, mode_t mode) {
  if (!original_creat64) {
    original_creat64 = (open_create_type)(dlsym(RTLD_NEXT, "creat64"));
  }
  int fd = original_creat64(path_name, flags, mode);
  return fd;
}

int openat(int dir_fd, const char *path_name, int flags, mode_t mode) {
  if(!original_openat) {
    original_openat = (open_at_type)(dlsym(RTLD_NEXT, "openat"));
  }
  int fd = original_openat(dir_fd, path_name, flags, mode);
  return fd;
}

int openat64(int dir_fd, const char *path_name, int flags, mode_t mode) {
  if (!original_openat64) {
    original_openat64 = (open_at_type)(dlsym(RTLD_NEXT, "openat64"));
  }
  int fd = original_openat64(dir_fd, path_name, flags, mode);
  return fd;
}

int dup(int old_fd) {
  if (!original_dup) {
    original_dup = (dup_type)(dlsym(RTLD_NEXT, "dup"));
  }

  int fd = original_dup(old_fd);
  return fd;
}

int dup2(int old_fd, int new_fd) {
  if (!original_dup2) {
    original_dup2 = (dup2_type)(dlsym(RTLD_NEXT, "dup2"));
  }
  int ret = original_dup2(old_fd, new_fd);
  return ret;
}

int close(int fd) {
  if (!original_close) {
    original_close = (close_type)(dlsym(RTLD_NEXT, "close"));
  }
  return original_close(fd);
}

FILE *fopen(const char *path, const char *mode) {
  FILE *fp = nullptr;
  if (!original_fopen) {
    original_fopen = (fopen_type)(dlsym(RTLD_NEXT, "fopen"));
  }
  int fd = -1;
  if((fp = original_fopen(path, mode)) != nullptr) {
    if((fd = fileno(fp)) != -1) {
      store_pageinfo(fd);
    }
  }
  return fp;
}

FILE *fopen64(const char *path, const char *mode) {
    FILE *fp = nullptr;
    if (!original_fopen64) {
      original_fopen64 = (fopen_type)(dlsym(RTLD_NEXT, "fopen64"));
    }
    int fd = -1;
    if((fp = original_fopen64(path, mode)) != nullptr) {
      if((fd = fileno(fp)) != -1) {
        store_pageinfo(fd);
      }
    }
    return fp;
}

int fclose(FILE *fp) {
  if (!original_fclose) {
    original_fclose = (fclose_type)(dlsym(RTLD_NEXT, "fclose"));
  }
  if (original_fclose) {
    return original_fclose(fp);
  }
  errno = EFAULT;
  return EOF;
}

void fcache_count_set_callback(fcache_count_callback_type call, void* profile) {
  callback = call;
  callback_profile = profile;
}

int insert_into_br_list(struct file_pageinfo *pi, struct byterange **brtail,
  size_t pos, size_t len) {
  struct byterange* tmp;
  tmp = static_cast<byterange*>(malloc(sizeof(*tmp)));
  if (!tmp) {
    return 0;
  }
      
  tmp->pos = pos;
  tmp->len = len;
  tmp->next = nullptr;

  if (pi->unmapped == nullptr) {
    pi->unmapped = tmp;
  } else if (*brtail != nullptr) {
    (*brtail)->next = tmp;
  }
  *brtail = tmp;
  return 1;
}

file_pageinfo* fd_get_pageinfo(int fd, struct file_pageinfo *pi) {
  void* file;
  struct byterange* br = nullptr;
  struct stat st;
  unsigned char* page_vec = nullptr;

  if (pi->fd != fd) {
    return nullptr;
  }
  pi->fd = fd;
  pi->unmapped = nullptr;

  if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode)) {
    return nullptr;
  }
  pi->size = st.st_size;
  pi->nr_pages = (st.st_size + PAGESIZE - 1) / PAGESIZE;

  fprintf(stderr, "fd_get_pageinfo(fd=%d): st.st_size=%lld, nr_pages=%lld\n",
          fd, (long long)st.st_size, (long long)pi->nr_pages);

  /* If size is 0, mmap() will fail. We'll keep the fd stored, anyway, to
   * make sure the newly written pages will be freed on close(). */
  if (pi->size == 0) {
    return pi;
  }

  /* If mmap() fails, we will probably have a file in write-only or
   * append-only mode. In this mode the caller will not be able to
   * bring in new pages anyway, but we'll record the current size */
  file = mmap(nullptr, st.st_size, PROT_NONE, MAP_SHARED, fd, 0);
  if (file == MAP_FAILED) {
    return pi;
  }

  page_vec = static_cast<unsigned char*>(
    calloc(sizeof(*page_vec), pi->nr_pages));
  if (!page_vec) {
    if (file) {
      munmap(file, st.st_size);
    }
    free(page_vec);
    return nullptr;
  }

  if (mincore(file, pi->size, page_vec) == -1) {
    if (file) {
      munmap(file, st.st_size);
    }
    free(page_vec);
    return nullptr;
  }

  munmap(file, st.st_size);
  file = nullptr;

  /* compute (byte) intervals that are *not* in the file system
   * cache, since we will want to free those on close() */
  pi->nr_pages_cached = pi->nr_pages;
  size_t i, start = 0;
  for(i = 0; i < pi->nr_pages; ++i) {
    if(!(page_vec[i] & 1))
      continue;
    if (start < i) {
      insert_into_br_list(pi, &br, start * PAGESIZE, (i - start) * PAGESIZE);
      pi->nr_pages_cached -= i - start;
    }
    start = i + 1;
  }
  /* Leftover interval: clear until end of file */
  if(start < pi->nr_pages) {
    insert_into_br_list(pi, &br, start * PAGESIZE, pi->size - start * PAGESIZE);
    pi->nr_pages_cached -= pi->nr_pages - start;
  }

  free(page_vec);
  return pi;
}

void free_br_list(struct byterange **br) {
  struct byterange *tmp;
  while(*br != nullptr) {
    tmp = *br;
    (*br) = tmp->next;
    free(tmp);
  }
  *br = nullptr;
}

void store_pageinfo(int fd) {
  sigset_t mask;
  sigset_t old_mask;

  if (fd >= max_fds) {
    return;
  }

  /* We might know something about this fd already, so assume we have missed
   * it being closed. */
  free_unclaimed_pages(fd);

  sigfillset(&mask);
  sigprocmask(SIG_BLOCK, &mask, &old_mask);

  pthread_mutex_lock(&fds_iter_lock);
  if (fds_lock == nullptr) {
    pthread_mutex_unlock(&fds_iter_lock);
    return;
  }
  pthread_mutex_lock(&fds_lock[fd]);
  pthread_mutex_unlock(&fds_iter_lock);

  /* Hint we'll be using this file only once;
   * the Linux kernel will currently ignore this */
  // fadv_noreuse(fd, 0, 0);

  fds[fd].fd = fd;
  if (!fd_get_pageinfo(fd, &fds[fd])) {
    fds[fd].fd = -1;
    goto out;
  }

  fprintf(stderr, "store_pageinfo(fd=%d): pages in cache: %zd/%zd (%.1f%%)  [filesize=%.1fK, "
    "pagesize=%dK]\n", fd, fds[fd].nr_pages_cached, fds[fd].nr_pages,
    fds[fd].nr_pages == 0 ? 0 : (100.0 * fds[fd].nr_pages_cached / fds[fd].nr_pages),
    1.0 * fds[fd].size / 1024, (int) PAGESIZE / 1024);

  out:
  pthread_mutex_unlock(&fds_lock[fd]);
  sigprocmask(SIG_SETMASK, &old_mask, NULL);

  return;
}

void free_unclaimed_pages(int fd) {
  struct stat st;
  sigset_t mask, old_mask;

  if (fd == -1 || fd >= max_fds) {
    return;
  }

  sigfillset(&mask);
  sigprocmask(SIG_BLOCK, &mask, &old_mask);

  pthread_mutex_lock(&fds_iter_lock);
  if (fds_lock == NULL) {
    pthread_mutex_unlock(&fds_iter_lock);
    return;
  }
  pthread_mutex_lock(&fds_lock[fd]);
  pthread_mutex_unlock(&fds_iter_lock);

  if (fds[fd].fd == -1) {
    goto out;
  }

  // sync_if_writable(fd);

  if(fstat(fd, &st) == -1) {
    goto out;
  }

  struct byterange *br;
  for (br = fds[fd].unmapped; br; br = br->next) {
      // fadv_dontneed(fd, br->pos, br->len, nr_fadvise);
  }

  /* Has the file grown bigger? */
  if (st.st_size > fds[fd].size) {
    // fadv_dontneed(fd, fds[fd].size, 0, nr_fadvise);
  }

  free_br_list(&fds[fd].unmapped);
  fds[fd].fd = -1;

  out:
  pthread_mutex_unlock(&fds_lock[fd]);
  sigprocmask(SIG_SETMASK, &old_mask, NULL);
}