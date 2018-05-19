//
// Created by skyitachi on 2018/5/20.
//
#ifndef LEARNUV_UTIL_H
#define LEARNUV_UTIL_H
#include "uv.h"
#include <cstdlib>

void *safe_malloc(size_t size);
void log_error(const char* prefix, int status);
void common_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void common_on_read_end();
void common_on_write_end(uv_write_t* req, int status);

#endif //LEARNUV_UTIL_H
