#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <sys/stat.h>

#include "fcache_count.hpp"

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


static __attribute__ ((constructor)) void init() {
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

int open(const char *path_name, int flags, mode_t mode) {
  if(!original_open) {
    original_open = (open_create_type)(dlsym(RTLD_NEXT, "open"));
  }
  int fd = original_open(path_name, flags, mode);
  return fd;
}

int open64(const char *path_name, int flags, mode_t mode) {
    if (!original_open64) {
      original_open64 = (open_create_type)(dlsym(RTLD_NEXT, "open64"));
    }
    int fd = original_open64(path_name, flags, mode);
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
  fp = original_fopen(path, mode);
  return fp;
}

FILE *fopen64(const char *path, const char *mode) {
    FILE *fp = nullptr;
    if (!original_fopen64) {
      original_fopen64 = (fopen_type)(dlsym(RTLD_NEXT, "fopen64"));
    }
    fp = original_fopen64(path, mode);
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