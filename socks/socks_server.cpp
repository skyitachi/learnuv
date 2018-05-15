#include "defs.h"
#include <stdlib.h>
#include <iostream>
#include <assert.h>

uv_loop_t *loop;

/**
 * 获取port字段（Big Endian, network byte order）
 * @param buf
 * @param offset
 * @return
 */
int readPort(const char *buf, ssize_t offset) {
  return ((unsigned char)buf[0 + offset] << 8) + (unsigned char)buf[1 + offset];
}

/**
 * parse addr info from socks client
 */
Addr* parseAddr(const char *buf, ssize_t offset) {
  Addr *addr = (Addr *)malloc(sizeof(Addr));
  addr->port = readPort(buf, offset);
  sprintf(addr->ip, "%d.%d.%d.%d", 
    (unsigned char)buf[2+offset],  
    (unsigned char)buf[3+offset],    
    (unsigned char)buf[4+offset],    
    (unsigned char)buf[5+offset]
  );
  return addr;
}

void alloc_buffer_client(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  Tunnel *tunnel;
  tunnel = CONTAINER_OF(handle, Tunnel, client.handle);
  assert(tunnel);
  printf("tunnel address is %p\n", tunnel);
  if (suggested_size > MAXSIZE) {
    printf("suggest size is %lu\n", suggested_size);
  }
  buf->base = tunnel->client_buf;
  buf->len = sizeof(tunnel->client_buf);
}

void alloc_buffer_server(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  Tunnel *tunnel;
  tunnel = CONTAINER_OF(handle, Tunnel, client.handle);
  assert(tunnel);
  printf("tunnel address is %p\n", tunnel);
  if (suggested_size > MAXSIZE) {
    printf("suggest size is %lu\n", suggested_size);
  }
  buf->base = tunnel->server_buf;
  buf->len = sizeof(tunnel->server_buf);
}


void on_client_write_end(uv_write_t *req, int status) {
  Tunnel *tunnel = CONTAINER_OF(req, Tunnel, client_write_req);

}

void on_server_write_end(uv_write_t *req, int status) {
  if (status < 0) {
    fprintf(stderr, "%s\n", uv_strerror(status));
  }
  Tunnel *tunnel = CONTAINER_OF(req, Tunnel, server_write_req);
  assert(tunnel);
  if (tunnel->status == CLOSE_CLIENT) {
//    if (tunnel->client != NULL) {
//      uv_close((uv_handle_t *) tunnel->client, NULL);
//      fprintf(stderr, "force close client connection\n");
//    }
//    if(tunnel->server != NULL) {
//      uv_close((uv_handle_t *) tunnel->server, NULL);
//      fprintf(stderr, "force close server connection\n");
//    }
  } else if (tunnel->status == SERVER_WRITE_END) {
    tunnel->status = CLIENT_WRITE_END;
  }
}

/**
 * read data from server connection
 * @param stream
 * @param nread
 * @param buf
 */
void on_server_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  // Note
  Tunnel *tunnel = CONTAINER_OF(stream, Tunnel, server.stream);
  assert(tunnel);
//  if (!tunnel->client) {
//    printf("client has closed\n");
//    return;
//  }
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Error on reading relay stream: %s.\n",
              uv_strerror(nread));
    }
    // close the connection
//    uv_close((uv_handle_t *) stream, NULL);
//    // 先关闭server
//    tunnel->server = NULL;
//    tunnel->status = SERVER_WRITE_END;
//    printf("real server closed\n");
    return;
  }
  // printf("server stream address: %p\n", stream);
  printf("read %ld size data from the server\n", nread);
  if (nread < 50) {
    printf("server data: %s\n", buf->base);
  }
//  if (strcasestr(buf->base, "Bad Request") != NULL) {
//    printf("Bad Request: %s\n", tunnel->header);
//    printf("response is %s\n", buf->base);
//    tunnel->status = CLOSE_CLIENT;
//  }
  uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
  write_req->data = (void *) tunnel;
  uv_write(write_req, &tunnel->client.stream, buf, 1, on_server_write_end);
}

/**
 * tunnel socket connect
 * @param req
 * @param status
 */
void on_server_connect(uv_connect_t *req, int status) {
  // read data from req then send to client
  Tunnel* tunnel = CONTAINER_OF(req, Tunnel, connect_req);
  assert(tunnel);
  printf("on_server_connect: tunnel address %p\n", tunnel);
  if (status < 0) {
    // 连接 server 失败
    fprintf(stderr, "%s\n", uv_strerror(status));
    free(tunnel);
    return;
  }
  // Note: 初始化 server.stream
//  tunnel->server.stream = *req->handle;
  tunnel->status = CONNECTED_TO_SERVER;
  printf("connect to real server correctly\n");
  // Note: some data in the buf
//  if (tunnel->offset) {
//    uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
//    uv_buf_t uvBuf = uv_buf_init(tunnel->buf, tunnel->offset);
//    tunnel->offset = 0;
//    write_req->data = (void *) tunnel;
//    uv_write(write_req, req->handle, &uvBuf, 1, on_write_end);
//  }
  uv_read_start(&tunnel->server.stream, alloc_buffer_server, on_server_read);
}

void on_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  Tunnel *tunnel = CONTAINER_OF(stream, Tunnel, client.stream);
  assert(tunnel);
  if (nread < 0) { // 客户端主动关闭连接
    if (nread != UV_EOF) {
      fprintf(stderr, "Error on reading client stream: %s.\n",
              uv_strerror(nread));
    }
    // close the connection
//    uv_close((uv_handle_t *) stream, NULL);
//    tunnel->client = NULL;
//    printf("client closed\n");
  } else {
    // printf("client_stream address %p\n", stream);
    assert(stream == &tunnel->client.stream);

    // socks4a client data
    // parse destination address
    // TODO: validate socks client auth information
    // socks4a
    if (buf->base[0] == 0x04 && buf->base[1] == 0x01) {
      char sendBuf[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};
      uv_buf_t uvBuf = uv_buf_init(sendBuf, sizeof(sendBuf));
      uv_write(&tunnel->client_write_req, stream, &uvBuf, 1, on_client_write_end);
      Addr *addr = parseAddr(buf->base, 2);
      tunnel->addr = *addr;
      printf("server addr is: %s:%d\n", addr->ip, addr->port);
      const char *username = (buf->base + 8);
      if (strlen(username) > 0) {
        printf("username is: %s\n", username);
      }
      struct sockaddr_in dest;
      uv_ip4_addr(addr->ip, addr->port, &dest);
      tunnel->status = CONNECTING_TO_SERVER;
      uv_tcp_init(loop, &tunnel->server.tcp);
      uv_tcp_connect(&tunnel->connect_req, &tunnel->server.tcp, (const struct sockaddr *) &dest, on_server_connect);
    } else {
      if (tunnel->status == CONNECTED_TO_SERVER) {
        // send server_buf data to server
        uv_buf_t tmpBuf = uv_buf_init(buf->base, nread);
        assert(0 == uv_write(&tunnel->client_write_req, &tunnel->server.stream, &tmpBuf, 1, on_client_write_end));
      } else if (tunnel->status == CONNECTING_TO_SERVER) {

        memcpy(tunnel->server_buf + tunnel->server_buf_offset, buf->base, nread);
        tunnel->server_buf_offset += nread;
        tunnel->server_buf[tunnel->server_buf_offset] = 0;
      } else {
        fprintf(stderr, "unexpected status: %d\n", tunnel->status);
        return;
      }
    }
  }
}

void on_new_client_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    std::cout << "new connection error: " << uv_strerror(status) << std::endl;
    return;
  }
  // 初始化connection
  Tunnel* tunnel = (Tunnel *)malloc(sizeof(Tunnel));
  printf("create tunnel success: %p\n", &tunnel);
  uv_tcp_init(loop, &tunnel->client.tcp);

  // accept
  if (!uv_accept(server, &tunnel->client.stream)) {
    // Note: important
    tunnel->status = TUNNEL_START;
    printf("client stream is %p\n", &tunnel->client.stream);
    int r = uv_read_start(&(tunnel->client.stream), alloc_buffer_client, on_client_read);
    if (r) {
      printf("read the client error: %s\n", uv_strerror(r));
    } else {
      printf("connect successfully\n");
    }
  } else {
    free(tunnel);
    printf("tunnel freed");
  }
}

void showUsage() {
  printf("Usage: socks -p 3000\n");
}

int main(int argc, char **argv) {
  const char *host = "127.0.0.1";
  int port = 3000;
  if (argc != 1 && argc != 3) {
    showUsage();
    exit(1);
  }
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-p")) {
      port = atoi(argv[i + 1]);
      break;
    }
  }
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  sockaddr_in addr;
  
  uv_ip4_addr(host, port, &addr);

  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0); // bind

  int r = uv_listen((uv_stream_t *)&server, 2, on_new_client_connection); // listen
  if (r) {
    printf("listen error: %s\n", uv_strerror(r));
  } else {
    printf("listen on %d successfully\n", port);
  }

  return uv_run(loop, UV_RUN_DEFAULT);
}
