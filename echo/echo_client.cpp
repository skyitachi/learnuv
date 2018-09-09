#include "util.h"

uv_loop_t* loop;
uv_pipe_t* pipe_stdin;
uv_pipe_t* pipe_stdout;

// Note: just print to the server
void on_read_connection(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, NULL);
    uv_close((uv_handle_t *) stream->data, NULL); // stdout
    uv_close((uv_handle_t *) pipe_stdin, NULL);
    return;
  }
  uv_stream_t *stream_stdout = (uv_stream_t *)stream->data;
  uv_write_t* write_req = (uv_write_t* ) safe_malloc(sizeof(uv_write_t));
  uv_buf_t tmpBuf = uv_buf_init(buf->base, nread);

  uv_write(write_req, stream_stdout, &tmpBuf, 1, common_on_write_end);
}

void on_read_stdin(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, NULL);
    uv_close((uv_handle_t *) stream->data, NULL); // connection
    uv_close((uv_handle_t *) pipe_stdout, NULL);
    return;
  }
  uv_stream_t* connection_stream = (uv_stream_t *)stream->data;

  uv_write_t* write_req = (uv_write_t* ) safe_malloc(sizeof(uv_write_t));
  uv_buf_t tmpBuf = uv_buf_init(buf->base, nread);

  uv_write(write_req, connection_stream, &tmpBuf, 1, common_on_write_end);
}

void on_connect(uv_connect_t* req, int status) {
  if (status < 0) {
    log_error("on_connect error: ", status);
    exit(1);
  }
  printf("connect successfully\n");
  // Note: write stdout
  pipe_stdout = (uv_pipe_t *) safe_malloc(sizeof(uv_pipe_t));
  check_uv(uv_pipe_init(loop, pipe_stdout, 0));
  check_uv(uv_pipe_open(pipe_stdout, 1));
  req->handle->data = pipe_stdout;

  uv_read_start(req->handle, common_alloc_buffer, on_read_connection);
  // Note: read stdin
  pipe_stdin = (uv_pipe_t *) safe_malloc(sizeof(uv_pipe_t));
  check_uv(uv_pipe_init(loop, pipe_stdin, 0));
  check_uv(uv_pipe_open(pipe_stdin, 0));
  printf("start read stdin\n");

  pipe_stdin->data = req->handle;
  check_uv(uv_read_start((uv_stream_t *) pipe_stdin, common_alloc_buffer, on_read_stdin));

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
  check_uv(uv_tcp_init(loop, client));

  uv_connect_t* connect_req = (uv_connect_t *) safe_malloc(sizeof(uv_connect_t));
  uv_tcp_connect(connect_req, client, (const sockaddr *)& addr, on_connect);
  uv_run(loop, UV_RUN_DEFAULT);
}
