//
// Created by skyitachi on 2018/6/3.
//
#include "util.h"
#include <math.h>

uv_loop_t *loop;
uv_buf_t buffer;
uv_fs_t read_req;
uv_fs_t close_req;
uv_fs_t open_req;
#define BUF_SIZE 4096

size_t count = 0;
uint64_t start;

void on_read(uv_fs_t *req) {
  if (req->result < 0) {
    fprintf(stderr, "read file error on_read\n");
    return;
  } if (req->result == 0) {
    printf("total file size: %lu\n", count);
    printf("time consume: %lf\n", floor((uv_hrtime() - start) * 1.0 / 1e6));
    uv_fs_close(loop, &close_req, open_req.result, NULL);
    return;
  } else {
    count += req->result;
    uv_fs_read(loop, &read_req, open_req.result, &buffer, 1, -1, on_read);
  }

}

void on_file_open(uv_fs_t* req) {
  if (req->result >= 0) {
    uv_fs_read(loop, &read_req, req->result, &buffer, 1, -1, on_read);
    return;
  }
  log_error("read file error: ", req->result);
}

int main(int argc, char **argv) {
  int buf_size = BUF_SIZE;
  if (argc < 2 || argc > 3) {
    printf("Usage: ./readline $file $buf_size\n");
    exit(1);
  }
  if (argc == 3) {
    buf_size = atoi(argv[2]);
  }
  char *base = (char *) malloc(buf_size);
  loop = uv_default_loop();
  buffer = uv_buf_init(base, buf_size);
  start = uv_hrtime();
  printf("file path is %s\n", argv[1]);
  check_uv(uv_fs_open(loop, &open_req, argv[1], O_RDONLY, 0, on_file_open));
  uv_run(loop, UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_req);
  uv_fs_req_cleanup(&read_req);

  free(buffer.base);
  return 0;
}
