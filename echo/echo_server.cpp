#include <uv.h>
#include <cstdlib>

uv_loop_t *loop;

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  char *base = (char *)malloc(suggested_size);
  if (base == NULL) {
    fprintf(stderr, "alloc_buffer failed\n");
    return;
  }
  buf->base = base;
  buf->len = suggested_size;
}

void on_write_end(uv_write_t* req, int status) {
  if (status < 0) {
    fprintf(stderr, "on_write_end write error: %s\n", uv_strerror(status));
    return;
  }
}

void on_data(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    // Note, Important: some clean work
    uv_close((uv_handle_t *) stream, NULL);
    printf("connection closed\n");
    return;
  }
  printf("receive data: %s", buf->base);
  uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
  uv_write(write_req, stream, buf, 1, on_write_end);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    printf("on_new_connection error %s\n", uv_strerror(status));
    return;
  }
  // Note: use malloc
  uv_tcp_t* client = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);

  if (!uv_accept(server, (uv_stream_t *)client)) {
    sockaddr_in clientAddr;
    char addr[32];
    int addrLen;
    uv_tcp_getpeername(client, (struct sockaddr *)&clientAddr, &addrLen);
    uv_ip4_name(&clientAddr, addr, sizeof(addr));
    printf("receive connections from %s:%d\n", addr, ntohs(clientAddr.sin_port));

    int r = uv_read_start((uv_stream_t *)client, alloc_buffer, on_data);
    if (r < 0) {
      fprintf(stderr, "read client data start error: %s\n", uv_strerror(r));
      return;
    } else {
      printf("read start successfully\n");
    }
  }
}

int main() {
  int port = 10007;
  const char *host = "0.0.0.0";
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  uv_tcp_bind(&server, (const sockaddr *)&addr, 0);

  int r = uv_listen((uv_stream_t *)&server, 1, on_new_connection);

  if (r) {
    printf("echo_server listen error: %s\n", uv_strerror(r));
  } else {
    printf("echo_sever listen on %s:%d successfully\n", host, port);
  }

  return uv_run(loop, UV_RUN_DEFAULT);
}
