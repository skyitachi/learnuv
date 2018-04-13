#include <stdio.h>
#include <uv.h>
#include <stdlib.h>
/**
 * setTimeout(cb, 1000)
 * uv_timer_start(&timer, timer_cb, timeout, 0)
 * setInterval(cb, 1000)
 * uv_timer_start(&timer, timer_cb, time_interval, time_interval)
 * @param handle
 */

void timer_cb(uv_timer_t *handle) {
  // stop the timer
  printf("hello world: %llu\n", handle->repeat);
}

void open_cb(uv_fs_t *req) {
  if (req->result >= 0) {
    printf("ok\n");
    return;
  }
  printf("failed\n");
}

int main() {
  int timeout = 1000;
  uv_loop_t *loop = (uv_loop_t* )malloc(sizeof(uv_loop_t));
//  uv_loop_init(loop);
//  uv_run(loop, UV_RUN_DEFAULT);
  uv_timer_t timer;
  uv_timer_init(uv_default_loop(), &timer);
//  int repeat = 1000; // setInterval interval
  int repeat = 0;
  // repeat = 0 -> no repeat as setTimeout
  // repeat > 0 -> repeat multiple times
  uv_timer_start(&timer, timer_cb ,timeout, repeat);
  uv_fs_t open_req;
  uv_fs_open(uv_default_loop(), &open_req, "/Users/skyitachi/tmp/addon/hello.cc", O_RDONLY, 0, open_cb);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  printf("Now quiting\n");
//  uv_loop_close(loop);
//  free(loop);
  return 0;
}
