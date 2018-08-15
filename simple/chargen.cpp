//
// Created by skyitachi on 2018/8/14.
//
#include "util.h"

#define BUFSIZE 4096

char globalBuf[BUFSIZE];

const char *message = "hello world\n";

static uint64_t transfered = 0;
static uint64_t ts = uv_hrtime();
typedef struct statistics {
  uint64_t start;
  uint64_t transfered;
} stats;

void on_close(uv_handle_t* handle) {
  printf("handle being closed\n");
}

void on_write_end(uv_write_t *req, int status) {
  stats *stat = (stats *)req->data;
  assert(stat != NULL);
  if (status < 0) {
    uint64_t now = uv_hrtime();
    // Note: why this not accurate
    printf("single transfer rate is: %lfMB/s\n", (stat->transfered * 1.0) / (now - stat->start) * 1e9 / (1024 * 1024));
    free(req->data);
    if (uv_is_writable(req->handle)) {
      log_error("on_write_end, unexpected error: ", status);
    } else {
      printf("client closed\n");
    }
    uv_close((uv_handle_t *)req->handle, on_close);
  }
  strcpy(globalBuf, message);
  stat -> transfered += sizeof(message);
  transfered += sizeof(message);
  uv_buf_t buf = uv_buf_init(globalBuf, sizeof(message));
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
  struct sockaddr_in addr;

  int namelen;
  check_uv(uv_tcp_getpeername(client, (struct sockaddr*) &addr, &namelen));

  if (addr.sin_family == AF_INET) {
    // remote_address的长度很重要
    char remote_address[INET_ADDRSTRLEN];
    check_uv(uv_ip4_name(&addr, remote_address, sizeof(remote_address)));
    printf("remote address is: %s:%d\n", remote_address, ntohs(addr.sin_port));
  } else {
    printf("ipv6 does not support\n");
  }

  //
  strcpy(globalBuf, message);
  uv_buf_t buf = uv_buf_init(globalBuf, sizeof(message));
  uv_write_t* req = (uv_write_t* )safe_malloc(sizeof(uv_write_t));
  stats* stat = (stats *)safe_malloc(sizeof(stats));
  stat -> start = uv_hrtime();
  stat -> transfered = 0;
  req->data = stat;
  uv_write(req, (uv_stream_t *) client, &buf, 1, on_write_end);
}

void on_timeout(uv_timer_t* timer) {
  uint64_t now = uv_hrtime();
  printf("transfer rate is %lfMB/s\n", (transfered * 1.0) / (now - ts) * 1e9 / 1024 / 1024);
  ts = now;
  transfered = 0;
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

  uv_timer_t *timer = (uv_timer_t *)safe_malloc(sizeof(uv_timer_t));
  uv_timer_init(uv_default_loop(), timer);
  check_uv(uv_timer_start(timer, on_timeout, 1000, 1000));
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
