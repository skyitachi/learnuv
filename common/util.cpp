//
// Created by skyitachi on 2018/5/20.
//
#include "util.h"

void *safe_malloc(size_t size) {
  void *ret = malloc(size);
  if (ret == NULL) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }
  return ret;
}

void log_error(const char* prefix, int status) {
  fprintf(stderr, "%s %s\n", prefix, uv_strerror(status));
}

void common_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  char *base = (char *)malloc(suggested_size);
  if (base == NULL) {
    fprintf(stderr, "alloc_buffer failed\n");
    return;
  }
  buf->base = base;
  buf->len = suggested_size;
}

void common_on_write_end(uv_write_t* req, int status) {
  if (status < 0) {
    fprintf(stderr, "on_write_end write error: %s\n", uv_strerror(status));
    return;
  }
}

void on_write_end_noop(uv_write_t* req, int status) {
  return;
}
