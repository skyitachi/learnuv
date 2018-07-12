//
// Created by skyitachi on 2018/7/8.
//
#include "util.h"
#include <math.h>
#include <vector>
int counter = 1;
int failed = 0;
uv_buf_t clientBuf;
uint64_t start = uv_hrtime();

void onConnect(uv_connect_t *req, int status) {
  if (status < 0) {
    counter -= 1;
    failed += 1;
    fprintf(stderr, "connection error: %s\n", uv_strerror(status));
    return;
  }
  uv_stream_t *client = req->handle;
  uv_write_t* write_req = (uv_write_t *) malloc(sizeof(uv_write_t));
  check_uv(uv_write(write_req, client, &clientBuf, 1, common_on_write_end));
}

void onReadData(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, NULL);
    return;
  }
  printf("read %zu size data\n", nread);
  counter -= 1;
  printf("counter is %d, failed is %d\n", counter, failed);
  if (!counter) {
    uint64_t now = uv_hrtime();
    printf("total time is: %f ms\n", (now - start) * 1.0 / 1e6);
  }
}

class Connection {
public:
  Connection(uv_loop_t *loop, sockaddr* addr): loop_(loop), addr_(addr) {
    client = (uv_tcp_t *)safe_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
  }
  ~Connection();
  void connect();

private:
  uv_tcp_t *client;
  uv_loop_t *loop_;
  uv_connect_t* connect_req;
  sockaddr *addr_;
};

Connection::~Connection() {
  printf("in the connection destroy\n");
}

void Connection::connect() {
  connect_req = (uv_connect_t *) safe_malloc(sizeof(uv_connect_t));
  check_uv(uv_tcp_connect(connect_req, client, addr_, onConnect));

  check_uv(uv_read_start((uv_stream_t *) client, common_alloc_buffer, onReadData));
}

int main(int argc, char* argv[]) {
  if (argc < 6) {
    fprintf(stderr, "Usage: client <host_ip> <port> <blocksize> <sessions> <time>\n");
    return 1;
  }
  char *host = argv[1];
  int port = atoi(argv[2]);
  int block_size = atoi(argv[3]);
  int sessions = atoi(argv[4]);
  counter = sessions;
  int timeout = atoi(argv[5]);
  struct sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  char *base = (char *)safe_malloc(block_size);
  clientBuf = uv_buf_init(base, block_size);
  for(int i = 0; i < block_size; i++) {
    base[i] = static_cast<char>(i % 128);
  }
  start = uv_hrtime();
  Connection** connections = (Connection **)safe_malloc(sizeof(Connection *) * sessions);
  for(int i = 0; i < sessions; i++) {
    Connection* c = new Connection(uv_default_loop(), (sockaddr *) &addr);
    connections[i] = c;
  }
  for(int i = 0; i < sessions; i++) {
    connections[i]->connect();
  }
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
