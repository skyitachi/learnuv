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

char globalBuf[8192];

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
  uv_os_fd_t os_fd;
  uv_fileno((const uv_handle_t* )client, &os_fd);
  
  int fd = os_fd;
  printf("fd is %d\n", fd);
  std::thread t1([fd](){
//    char buf[1024];
//    int size = ::read(fd, buf, sizeof(buf));
//    if (size >= 0) {
//      std::cout << "receive data " << std::string(buf, size) << std::endl;
//    } else {
//      std::cerr << "read error happens " << strerror(errno) << std::endl;
//    }
    uv_tcp_t* child = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_loop_t* loop1 = uv_loop_new();
    uv_tcp_init(loop1, child);
    uv_tcp_open(child, fd);
    uv_read_start(
      (uv_stream_t *)child,
      [](uv_handle_t *handle, size_t suggest_size, uv_buf_t* buf){
        *buf = uv_buf_init(globalBuf, sizeof(globalBuf));
      },
      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        if (nread < 0) {
          fprintf(stdout, "child thread read error %s\n", uv_strerror(nread));
          return;
        } else {
          std::cout << "curren thread " << std::this_thread::get_id() << std::endl;
          fprintf(stdout, "receive data %s\n", buf->base);
          uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
          uv_write(req, stream, buf, 1, [](uv_write_t* req, int status) {
            free(req);
            if (status < 0) {
              fprintf(stderr, "child write error %s\n", uv_strerror(status));
              return;
            }
          });
        }
      });
    uv_run(loop1, UV_RUN_DEFAULT);
  });
  t1.join();
  
}

int main() {
  sockaddr_in sockaddrIn;
  std::cout << "main thread " << std::this_thread::get_id() << std::endl;
  uv_ip4_addr("127.0.0.1", 3000, &sockaddrIn);
  uv_tcp_t *server = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), server);
  uv_tcp_bind(server, (const sockaddr* )&sockaddrIn, 0);
  uv_listen((uv_stream_t* )server, 1024, on_new_connection);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
