#include "util.h"

uv_loop_t* loop;
//uv_buf_t tmpBuf;
char line[1024];

// Note: just print to the server
void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  printf("%s", buf->base);
}

void on_connect(uv_connect_t* req, int status) {
  if (status < 0) {
    log_error("on_connect error: ", status);
    exit(1);
  }
  printf("connect successfully\n");
  uv_read_start(req->handle, common_alloc_buffer, on_read);
  while(true) {
    scanf("%s", line);
    printf("stdin: %s\n", line);
    uv_write_t* write_req = (uv_write_t* ) safe_malloc(sizeof(uv_write_t));
    uv_buf_t tmpBuf = uv_buf_init(line, strlen(line));
    uv_write(write_req, req->handle, &tmpBuf, 1, common_on_write_end);
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: ./echo_client ${host} ${port}\n");
    exit(1);
  }
  char *host = argv[1];
  int port = atoi(argv[2]);
  struct sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  loop = uv_default_loop();
  uv_tcp_t* client = (uv_tcp_t*) safe_malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);

  uv_connect_t* connect_req = (uv_connect_t *) safe_malloc(sizeof(uv_connect_t));
  uv_tcp_connect(connect_req, client, (const sockaddr *)& addr, on_connect);
  uv_run(loop, UV_RUN_DEFAULT);
}
