//
// Created by skyitachi on 2018/9/5.
//

#include "util.h"
#include "Codec.h"
#include "buffer/Buffer.h"

Codec codec;
Buffer buffer;
uv_pipe_t* stdinPipe;

char sendBuf[1024];
int seq = 0;
typedef struct Stat {
  int done;
  Stat(int d): done(d) {}
} Stat;

static void on_shutdown(uv_shutdown_t*, int status);

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
  int contentLen = strlen(msg) + 4;
  printf("contentLen %d\n", contentLen);
  codec.encode(&buffer, msg);

  int step = 2, i = 0;
  for(i = 0; i < contentLen; i += step) {
    if (i + step > contentLen) {
      sendRawContent(buffer.peek() + i, contentLen - i, stream, 1);
    } else {
      sendRawContent(buffer.peek() + i, step, stream, 0);
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

static void on_read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    if (nread == UV_EOF) {
      printf("stdin_closed\n");
      return;
    }
    log_error("read error: ", nread);
  } else if (nread == 0) {

  }
  uv_stream_t* tcpClient = (uv_stream_t *) stream->data;
  codec.encode(&buffer, buf->base);
  sendRawContent(buffer.peek(), sizeof(uint32_t) + nread, tcpClient, 1);
}

static void on_read_tcp(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  printf("in the read_tcp\n");
  if (nread < 0) {
    if (nread == UV_EOF) {
      uv_close((uv_handle_t *)stdinPipe, NULL);
      return;
    }
    log_error("read_tcp error", nread);
    return;
  } else if (nread == 0) {
    return;
  }
  printf("receive message from others: %s", buf->base);
}

void on_connected(uv_connect_t* req, int status) {
  if (status < 0) {
    log_error("connect error: ", status);
    return;
  }
  printf("connected to server\n");
  stdinPipe = (uv_pipe_t *)safe_malloc(sizeof(uv_pipe_t));
  uv_pipe_init(uv_default_loop(), stdinPipe, 0);
  check_uv(uv_pipe_open(stdinPipe, 0));

  stdinPipe->data = req->handle;
  check_uv(uv_read_start((uv_stream_t *)stdinPipe, common_alloc_buffer, on_read_stdin));
  printf("after uv_read_start\n");

  check_uv(uv_read_start(req->handle, common_alloc_buffer, on_read_tcp));
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

