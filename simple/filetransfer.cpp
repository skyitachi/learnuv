//
// Created by skyitachi on 2018/8/30.
//
// Note: on_write_end里再去 uv_fs_read 即可
#include "util.h"
#include <unistd.h>
#define FILE_BUF 8192
uv_buf_t buffer;
char buf[FILE_BUF];

typedef struct  context {
  uv_fs_t *open_req;
  uv_fs_t *read_req;
  uv_tcp_t *server;
} context;

void on_read(uv_fs_t *req);
void on_fs_close(uv_fs_t *req);

void on_tcp_close(uv_handle_t* handle);

void on_shutdown(uv_shutdown_t* req, int status) {
  if (status < 0) {
    log_error("shutdown error: ", status);
    return;
  }
  context* ctx = (context *)req->data;
  uv_fs_t* close_req = (uv_fs_t *)safe_malloc(sizeof(uv_fs_t));
  close_req->data = ctx;
  uv_fs_close(uv_default_loop(), close_req, ctx->open_req->result, on_fs_close);
  printf("shutdown right\n");
}

void on_write_end(uv_write_t *req, int status) {
  if (status < 0) {
    fprintf(stderr, "on_write_end write error: %s\n", uv_strerror(status));
    return;
  }
  context* ctx = (context *)req->data;
  assert(ctx);
  uv_fs_read(uv_default_loop(), ctx->read_req, ctx->open_req->result, &buffer, 1, -1, on_read);
}

void on_tcp_close(uv_handle_t *handle) {
  context* ctx = (context *)handle->data;
  uv_fs_req_cleanup(ctx->open_req);
  uv_fs_req_cleanup(ctx->read_req);
  free(ctx);
  printf("clean done\n");

}

void on_fs_close(uv_fs_t *req) {
  context* ctx = (context *)req->data;
  assert(ctx);
  printf("file %zd closed\n", ctx->open_req->result);
  uv_close((uv_handle_t*) ctx->server, on_tcp_close);
}

void on_read(uv_fs_t *req) {
  context *ctx = (context *)req->data;
  assert(ctx->read_req == req);
  if (req->result < 0) {
    log_error("read file error", req->result);
    uv_fs_req_cleanup(ctx->read_req);
    uv_fs_req_cleanup(ctx->open_req);
    return;
  } if (req->result == 0) {
    printf("read file end\n");
    uv_stream_t *server = (uv_stream_t *)ctx->server;
    assert(server);
    uv_shutdown_t* shutdown = (uv_shutdown_t* )safe_malloc(sizeof(uv_shutdown_t));
    shutdown->data = ctx;
    uv_shutdown(shutdown, server, on_shutdown);

    printf("shutdown\n");
  } else {
    uv_stream_t *server = (uv_stream_t *)ctx->server;
    assert(server);

    uv_write_t* wReq = (uv_write_t *) safe_malloc(sizeof(uv_write_t));
    // Note: 不能直接使用 buffer
    char tBuf[FILE_BUF];
    memcpy(tBuf, buffer.base, req->result);
    wReq->data = ctx;
    tBuf[req->result] = 0;
    uv_buf_t tcpBuf = uv_buf_init(tBuf, req->result + 1);
    uv_write(wReq, server, &tcpBuf, 1, on_write_end);
  }
}

void on_file_open(uv_fs_t* req) {
  if (req->result >= 0) {
    printf("file descriptor is %zd\n", req->result);
    // client stream
    context *ctx = (context *) req->data;
    assert(ctx->open_req == req);
    uv_fs_t *read_req = (uv_fs_t *)safe_malloc(sizeof(uv_fs_t));
    ctx->read_req = read_req;
    read_req->data = ctx;
    uv_fs_read(uv_default_loop(), read_req, req->result, &buffer, 1, -1, on_read);
    return;
  }
  // clean open_req
  uv_fs_req_cleanup(req);
  context *ctx = (context *) req->data;
  free(ctx);
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
  uv_fs_t *open_req = (uv_fs_t *)safe_malloc(sizeof(uv_fs_t));
  // 上下文传递
  context *ctx = (context *)safe_malloc(sizeof(context));
  client->data = ctx;
  ctx->open_req = open_req;
  ctx->server = client;
  open_req->data = ctx;
  check_uv(uv_fs_open(uv_default_loop(), open_req, filename, O_RDONLY, 0, on_file_open));
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

