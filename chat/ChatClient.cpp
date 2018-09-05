//
// Created by skyitachi on 2018/9/5.
//

#include "util.h"
char sendBuf[1024];

void writeMessage(const char *msg, uv_stream_t* stream) {
  uint32_t len = htonl(strlen(msg));
  uv_buf_t ub = uv_buf_init(sendBuf, strlen(msg) + 4);
  memcpy(sendBuf, (char* )&len, 4);
  memcpy(sendBuf + 4, msg, strlen(msg));
  printf("length is %d\n", strlen(msg));
  uv_write_t req;
  uv_write(&req, stream, &ub, 1, common_on_write_end);
}

void on_connected(uv_connect_t* req, int status) {
  if (status < 0) {
    log_error("connect error: ", status);
    return;
  }
  printf("connected to server\n");
  writeMessage("hello world", req->handle);
}

int main() {
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 11111, &addr);
  uv_tcp_t client;
  uv_tcp_init(uv_default_loop(), &client);
  uv_connect_t connReq;
  uv_tcp_connect(&connReq, &client, (const sockaddr *)&addr, on_connected);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

