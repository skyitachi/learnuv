//
// Created by skyitachi on 2018/9/5.
//

#include "util.h"
char sendBuf[1024];

int seq = 0;
typedef struct Stat {
  int done;
  Stat(int d): done(d) {}
} Stat;

static void on_shutdown(uv_shutdown_t*, int status);

void writeMessage(const char *msg, uv_stream_t* stream) {
  uint32_t len = htonl(strlen(msg));
  uv_buf_t ub = uv_buf_init(sendBuf, strlen(msg) + 4);
  memcpy(sendBuf, (char* )&len, 4);
  memcpy(sendBuf + 4, msg, strlen(msg));
  printf("length is %zd\n", strlen(msg));
  uv_write_t req;
  uv_write(&req, stream, &ub, 1, common_on_write_end);
}

static void on_write_end(uv_write_t* req, int status) {
  if (status < 0) {
    log_error("write error: ", status);
    return;
  }
  Stat* stat = (Stat *)req->data;
  printf("on_write_end: done %d, seq %d, stat %p \n", stat->done, seq++, stat);
  if (stat->done) {
    // Note: 会导致segmentation fault
    // uv_shutdown_t shutdown;
//    uv_shutdown_t* shutdown = (uv_shutdown_t*)safe_malloc(sizeof(uv_shutdown_t));
//    uv_shutdown(shutdown, req->handle, on_shutdown);
  }
}

const char* encode(const char *msg) {
  int len = strlen(msg);
  printf("message length: %d\n", len);
  char *encodeBuf = (char *)safe_malloc(len + 4 + 1);
  uint32_t nlen = htonl(len);
  memcpy(encodeBuf, (char *)&nlen, 4);
  memcpy(encodeBuf + 4, msg, len);
  encodeBuf[len + 4] = 0;
  return encodeBuf;
}

void sendRawContent(const char *buf, ssize_t len, uv_stream_t* stream, int done) {
  // TODO: uv_write_t req; 两者有何区别
  uv_write_t* req = (uv_write_t *)safe_malloc(sizeof(uv_write_t));
  memcpy(sendBuf, buf, len);
  uv_buf_t ub = uv_buf_init(sendBuf, len);
  printf("send content:  %p, buf address %p\n", req, (void *)&ub);
  Stat *stat = new Stat(done);
  req->data = stat;
  printf("send stat addr %p, done: %d\n", stat, stat->done);
  uv_write(req, stream, &ub, 1, on_write_end);
}

// send message by one byte
void test(const char *msg, uv_stream_t * stream) {
  const char *buf = encode(msg);
  int contentLen = strlen(msg) + 4;
  printf("contentLen %d\n", contentLen);
  int step = 2, i = 0;
  for(i = 0; i < contentLen; i += step) {
    if (i + step > contentLen) {
      sendRawContent(buf + i, contentLen - i, stream, 1);
    } else {
      sendRawContent(buf + i, step, stream, 0);
    }
  }
}

static void on_shutdown(uv_shutdown_t *req, int status) {
  if (status < 0) {
    log_error("shutdown error: ", status);
    return;
  }
  printf("shutdown ok\n");
}

static void on_close(uv_handle_t* handle) {
  printf("in the close\n");
}

void on_connected(uv_connect_t* req, int status) {
  if (status < 0) {
    log_error("connect error: ", status);
    return;
  }
  printf("connected to server\n");
  test("hello world", req->handle);
}

int main() {
  printf("Stat size: %d\n", sizeof(Stat));
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 11111, &addr);
  uv_tcp_t client;
  uv_tcp_init(uv_default_loop(), &client);
  uv_tcp_nodelay(&client, 1);
  uv_connect_t connReq;
  uv_tcp_connect(&connReq, &client, (const sockaddr *)&addr, on_connected);
  check_uv(uv_run(uv_default_loop(), UV_RUN_DEFAULT));
}

