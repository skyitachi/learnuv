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

const char* encode(const char *msg) {
  int len = strlen(msg);
  char *encodeBuf = (char *)safe_malloc(len +  + 1);
  uint32_t nlen = htonl(strlen(msg));
  memcpy(encodeBuf, (char *)&nlen, 4);
  memcpy(encodeBuf + 4, msg, strlen(msg));
  encodeBuf[len + 4] = 0;
  return encodeBuf;
}

void sendRawContent(const char *buf, ssize_t len, uv_stream_t* stream) {
  uv_write_t req;
  uv_buf_t ub = uv_buf_init(sendBuf, len);
  memcpy(sendBuf, buf, len);
  uv_write(&req, stream, &ub, 1, common_on_write_end);
}

// send message by one byte
void test(const char *msg, uv_stream_t * stream) {
  const char *buf = encode(msg);
  int contentLen = strlen(buf);
  for(int i = 0; i < contentLen; i++) {
    sendRawContent(buf + i, 1, stream);
  }
}

void on_connected(uv_connect_t* req, int status) {
  if (status < 0) {
    log_error("connect error: ", status);
    return;
  }
  printf("connected to server\n");
  test("hello world", req->handle);
//  writeMessage("hello world", req->handle);
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

