//
// Created by skyitachi on 2018/9/3.
//

#include "util.h"
#include "buffer/Buffer.h"
#include "connection.h"
#include <uv.h>
#include <string>
#include <vector>

std::vector<Connection *> conns;
Codec* codec;
Buffer buf;
int connections = 0;

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    if (nread == UV_EOF) {
      Connection *conn = (Connection *)stream->data;
      printf("client closed: %d\n", conn->id());
      for(auto it = conns.begin(); it != conns.end(); it++) {
        if (*it == conn) {
          conns.erase(it);
          delete(conn);
          break;
        }
      }
      return;
    }
    log_error("read error: ", nread);
  } else if (nread == 0) {
    printf("in the read\n");
    return;
  }
  Connection* conn = (Connection *) stream->data;
  codec->on_uv_read(nread, buf);
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
  conns.push_back(conn);
  check_uv(uv_accept(server, (uv_stream_t *) client));
  client->data = conn;
  uv_read_start((uv_stream_t *)client, common_alloc_buffer, on_read);
}

void onStringMessageBack(const std::string& msg) {
  printf("receive msg: %s\n", msg.c_str());
  int len = codec->encode(&buf, msg.c_str());
  printf("buffer size: %d\n", buf.readableBytes());
  for(auto conn: conns) {
    conn->send(buf.peek(), len);
  }
}

int main(int argc, char **argv) {
  codec = new Codec();
  codec->setMessageCallback(onStringMessageBack);
  uv_tcp_t server;
  uv_tcp_init(uv_default_loop(), &server);
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 11111, &addr);
  uv_tcp_bind(&server, (const sockaddr *)&addr, 0);
  uv_listen((uv_stream_t *) &server, 128, on_connection);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

