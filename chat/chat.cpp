//
// Created by skyitachi on 2018/9/3.
//

#include "util.h"
#include <uv.h>
#include "connection.h"

int connections = 0;

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  Connection* conn = (Connection *) stream->data;
  conn->codec_.handleRead(nread, buf);
}

void on_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    log_error("listen error: ", status);
    return;
  }
  printf("new connection comes\n");
  uv_tcp_t *client = (uv_tcp_t *)safe_malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), client);
  Connection* conn = new Connection(connections++, client);
  check_uv(uv_accept(server, (uv_stream_t *) client));
  client->data = conn;
  uv_read_start((uv_stream_t *)client, common_alloc_buffer, on_read);
}

int main(int argc, char **argv) {
  uv_tcp_t server;
  uv_tcp_init(uv_default_loop(), &server);
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 11111, &addr);
  uv_tcp_bind(&server, (const sockaddr *)&addr, 0);
  uv_listen((uv_stream_t *) &server, 128, on_connection);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

