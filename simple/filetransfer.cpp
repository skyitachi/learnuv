//
// Created by skyitachi on 2018/8/30.
//
// Note: on_write_end里再去 uv_fs_read 即可
#include "util.h"
#define FILE_BUF 8192
uv_buf_t buffer;
uv_fs_t open_req;
uv_fs_t read_req;
char buf[FILE_BUF];

void on_read(uv_fs_t *req);

void on_shutdown(uv_shutdown_t* req, int status) {
  if (status < 0) {
    log_error("shutdown error: ", status);
    return;
  }
  printf("shutdown right\n");
}

void on_write_end(uv_write_t *req, int status) {
  if (status < 0) {
    fprintf(stderr, "on_write_end write error: %s\n", uv_strerror(status));
    return;
  }
  uv_fs_read(uv_default_loop(), &read_req, open_req.result, &buffer, 1, -1, on_read);
}

void on_read(uv_fs_t *req) {
  if (req->result < 0) {
    log_error("read file error", req->result);
    return;
  } if (req->result == 0) {
    printf("read file end\n");
    uv_fs_t* close_req = (uv_fs_t *)safe_malloc(sizeof(uv_fs_t));
    uv_fs_close(uv_default_loop(), close_req, open_req.result, NULL);
    uv_fs_req_cleanup(close_req);
    uv_stream_t *server = (uv_stream_t *)req->data;
    assert(server);
    uv_shutdown_t* shutdown = (uv_shutdown_t* )safe_malloc(sizeof(uv_shutdown_t));
    uv_shutdown(shutdown, server, on_shutdown);
    printf("shutdown\n");
  } else {
    uv_stream_t *server = (uv_stream_t *)req->data;
    assert(server);

    uv_write_t* wReq = (uv_write_t *) safe_malloc(sizeof(uv_write_t));
    // Note: 不能直接使用 buffer
    char tBuf[FILE_BUF];
    memcpy(tBuf, buffer.base, req->result);
    tBuf[req->result] = 0;
    uv_buf_t tcpBuf = uv_buf_init(tBuf, req->result + 1);
    uv_write(wReq, server, &tcpBuf, 1, on_write_end);
  }
}

void on_file_open(uv_fs_t* req) {
  if (req->result >= 0) {
    printf("file descriptor is %zd\n", req->result);
    read_req.data = req->data;
    uv_fs_read(uv_default_loop(), &read_req, req->result, &buffer, 1, -1, on_read);
    return;
  }
  log_error("open file error: ", req->result);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    log_error("connection error", status);
    return;
  }
  char *filename = (char *)server->data;
  printf("filename is %s\n", filename);
  uv_tcp_t *client = (uv_tcp_t* )safe_malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), client);
  check_uv(uv_accept(server, (uv_stream_t *)client));
  open_req.data = client;
  check_uv(uv_fs_open(uv_default_loop(), &open_req, filename, O_RDONLY, 0, on_file_open));
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: fs <port> <file>\n");
    exit(1);
  }
  int port = atoi(argv[1]);
  struct sockaddr_in* addr = (struct sockaddr_in *)safe_malloc(sizeof(struct sockaddr_in));
  uv_ip4_addr("0.0.0.0", port, addr);
  uv_tcp_t server;
  buffer = uv_buf_init(buf, FILE_BUF);
  check_uv(uv_tcp_init(uv_default_loop(), &server));
  check_uv(uv_tcp_bind(&server, (struct sockaddr* )addr, 0));
  server.data = argv[2];
  check_uv(uv_listen((uv_stream_t *)&server, 1, on_new_connection));

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

