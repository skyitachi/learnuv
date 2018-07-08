#include "util.h"
#define PINGPONG_BUF_SIZE 2 << 14

uv_loop_t* loop;
uv_buf_t msgBuf;

size_t buf_size = PINGPONG_BUF_SIZE;

void alloc_buffer(uv_handle_t *handle, size_t suggest_size, uv_buf_t* buf) {
  buf->base = msgBuf.base;
  buf->len = msgBuf.len;
}

void on_read_client(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) client, NULL);
    return;
  }
  printf("read %zu size data\n", nread);
  // Note: send the pong
  uv_write_t* write_req = (uv_write_t* ) safe_malloc(sizeof(uv_write_t));
  uv_buf_t write_buf = uv_buf_init(buf->base, nread);
  uv_write(write_req, client, &write_buf, 1, common_on_write_end);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    log_error("on_new_connection error", status);
    return;
  }
  printf("connection comes\n");
  uv_tcp_t* client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if (!uv_accept(server, (uv_stream_t *)client)) {
    // Note: read the message
    check_uv(uv_read_start((uv_stream_t *) client, alloc_buffer, on_read_client));
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: server <address> <port>\n");
    return 1;
  }
  int port = atoi(argv[2]);
  char *base = (char *) malloc(buf_size);
  msgBuf = uv_buf_init(base, buf_size);

  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  
  sockaddr_in addr;
  uv_ip4_addr(argv[1], port, &addr);
  uv_tcp_bind(&server, (const sockaddr *) &addr, 0);

  check_uv(uv_listen((uv_stream_t *)&server, 1, on_new_connection));

  return uv_run(loop, UV_RUN_DEFAULT);
}

