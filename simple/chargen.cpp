//
// Created by skyitachi on 2018/8/14.
//

#include "util.h"

#define BUFSIZE 4096

char globalBuf[BUFSIZE];

const char *message = "hello world\n";

static int64_t transfered = 0;

void on_close(uv_handle_t* handle) {
  printf("handle being closed, send %lld bytes data\n", transfered);
}

void on_write_end(uv_write_t *req, int status) {
  transfered += strlen(message);
  if (status < 0) {
    log_error("on_write_end: ", status);
    uv_close((uv_handle_t *)req->handle, on_close);
  }
  strcpy(globalBuf, message);
  uv_buf_t buf = uv_buf_init(globalBuf, sizeof(message));
  if (!uv_is_writable(req->handle)) {
    printf("stream not writable\n");
    uv_close((uv_handle_t *)req->handle, on_close);
    return;
  }
  uv_write(req, req->handle, &buf, 1, on_write_end);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    log_error("on_new_connection: ", status);
    return;
  }
  uv_tcp_t *client = (uv_tcp_t *)safe_malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), client);
  check_uv(uv_accept(server, (uv_stream_t *)client));
  printf("connection fd %d\n", client->accepted_fd);

  struct sockaddr_storage storage;
  struct sockaddr *const addr = reinterpret_cast<struct sockaddr*>(&storage);
  int namelen;
  check_uv(uv_tcp_getpeername(client, addr, &namelen));

  char remote_address[512];
  printf("ss_family: %d\n", storage.ss_family);
  if (addr->sa_family == AF_INET) {
    const struct sockaddr_in* a4 = (const struct sockaddr_in *)addr;
    check_uv(uv_ip4_name((const struct sockaddr_in *) addr, remote_address, sizeof(remote_address)));
    
    printf("remote address is: %s:%d\n", remote_address, ntohs(a4->sin_port));

  } else if (addr->sa_family == AF_INET6) {
    check_uv(uv_ip6_name((const struct sockaddr_in6 *) addr, remote_address, sizeof(remote_address)));
    printf("client connection: %s\n", remote_address);
  }

  //
  strcpy(globalBuf, message);
  uv_buf_t buf = uv_buf_init(globalBuf, sizeof(message));
  uv_write_t req;
  uv_write(&req, (uv_stream_t *) client, &buf, 1, on_write_end);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: chargen <port>\n");
    exit(1);
  }
  int port = atoi(argv[1]);
  struct sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", port, &addr);
  uv_tcp_t *server = (uv_tcp_t *)safe_malloc(sizeof(uv_tcp_t));

  uv_tcp_init(uv_default_loop(), server);
  uv_tcp_bind(server, (const struct sockaddr*) &addr, 0);

  check_uv(uv_listen((uv_stream_t *) server, 0, on_new_connection));

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
