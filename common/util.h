//
// Created by skyitachi on 2018/5/20.
//
#ifndef LEARNUV_UTIL_H
#define LEARNUV_UTIL_H
#include "uv.h"
#include <sys/socket.h>
#include <cstdlib>

#define check_uv(status) \
  do { \
    int code = (status); \
    if(code < 0){ \
      fprintf(stderr, "%s: %s\n", uv_err_name(code), uv_strerror(code)); \
      exit(code); \
    } \
  } while(0)

void *safe_malloc(size_t size);
void log_error(const char* prefix, int status);
void common_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void common_on_read_end();
void common_on_write_end(uv_write_t* req, int status);
void on_write_end_noop(uv_write_t* req, int status);

#endif //LEARNUV_UTIL_H
