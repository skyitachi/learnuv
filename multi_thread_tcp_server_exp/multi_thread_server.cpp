//
// Created by skyitachi on 2019-06-18.
//
// 一个失败的尝试, 只能从uv_accept中进行线程创建？？？

#include <uv.h>
#include <thread>

void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    fprintf(stdout, "listen error %s\n", uv_strerror(status));
    return;
  }
  std::thread t1([server]() {
    uv_loop_t *loop = uv_loop_new();
    uv_tcp_t* client = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    uv_run(loop, UV_RUN_ONCE);
    printf("before in another thread accept\n");
    int r = uv_accept(server, (uv_stream_t* )client);
    if (r < 0) {
      fprintf(stdout, "accept error %s\n", uv_strerror(r));
      return;
    }
    printf("accepted in another thread ok\n");
  });
  t1.join();
}

int main() {
  sockaddr_in sockaddrIn;
  uv_ip4_addr("127.0.0.1", 3000, &sockaddrIn);
  uv_tcp_t *server = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), server);
  uv_tcp_bind(server, (const sockaddr* )&sockaddrIn, 0);
  uv_listen((uv_stream_t* )server, 1024, on_new_connection);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}