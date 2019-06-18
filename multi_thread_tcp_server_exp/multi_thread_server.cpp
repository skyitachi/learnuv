//
// Created by skyitachi on 2019-06-18.
//
// 一个失败的尝试, 只能从uv_accept中进行线程创建？？？
// round 2: 使用raw fd in other thread

#include <uv.h>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <iostream>

void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    fprintf(stdout, "listen error %s\n", uv_strerror(status));
    return;
  }
  uv_loop_t* loop = uv_default_loop();
  uv_tcp_t* client = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  int r = uv_accept(server, (uv_stream_t* )client);
  if (r < 0) {
    fprintf(stdout, "accept error %s\n", uv_strerror(r));
    return;
  }
  int fd = client->accepted_fd;
  std::thread t1([fd](){
    char buf[1024];
    int size = ::read(fd, buf, sizeof(buf));
    if (size >= 0) {
      std::cout << "receive data " << std::string(buf, size) << std::endl;
    } else {
      std::cerr << "read error happens " << strerror(errno) << std::endl;
    }
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