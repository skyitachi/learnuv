#include <uv.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MAXSIZE 4096
#define MAXCONNECTION 128
#define REQUEST_SIZE 512

uv_loop_t *loop;

typedef struct Addr {
  char ip[16];
  int port;
} Addr;

typedef enum Status {
  TUNNEL_START,
  SUCCESS,
  CLOSE_CLIENT,
  BAD_REQUEST,
  SERVER_WRITE_END,
  CLIENT_WRITE_END
} Status;

typedef struct Tunnel {
  int offset;
  Addr* addr;
  char buf[MAXSIZE];
  char header[REQUEST_SIZE];
  char response[REQUEST_SIZE];
  Status status;
  uv_stream_t* client; // stream between client and proxy
  uv_stream_t* server; // stream between server and proxy
} Tunnel;


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

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  if (suggested_size > MAXSIZE) {
    printf("suggest size is %lu\n", suggested_size);
  }
  *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void show_binary(const char *s, ssize_t nread) {
  for(ssize_t i = 0; i < nread; i++) {
    printf("0x%x ", (unsigned char)s[i]);
  }
  printf("\n");
}

void on_write_end(uv_write_t *req, int status) {
  if (status < 0) {
    fprintf(stderr, "%s\n", uv_strerror(status));
  }
  Tunnel *tunnel = (Tunnel *) req->data;
  assert(tunnel);
  if (tunnel->status == CLOSE_CLIENT) {
    if (tunnel->client != NULL) {
      uv_close((uv_handle_t *) tunnel->client, NULL);
      fprintf(stderr, "force close client connection\n");
    }
    if(tunnel->server != NULL) {
      uv_close((uv_handle_t *) tunnel->server, NULL);
      fprintf(stderr, "force close server connection\n");
    }
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
  Tunnel *tunnel = (Tunnel *) stream->data;
  assert(tunnel);
  if (!tunnel->client) {
    printf("client has closed\n");
    return;
  }
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Error on reading relay stream: %s.\n",
              uv_strerror(nread));
    }
    // close the connection
    uv_close((uv_handle_t *) stream, NULL);
    // 先关闭server
    tunnel->server = NULL;
    tunnel->status = SERVER_WRITE_END;
    printf("real server closed\n");
    return;
  }
  // printf("server stream address: %p\n", stream);
  printf("read %ld size data from the server\n", nread);
  if (nread < 50) {
    printf("server data: %s\n", buf->base);
  }
  if (strcasestr(buf->base, "Bad Request") != NULL) {
    printf("Bad Request: %s\n", tunnel->header);
    printf("response is %s\n", buf->base);
    tunnel->status = CLOSE_CLIENT;
  }
  uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
  write_req->data = (void *) tunnel;
  uv_write(write_req, tunnel->client, buf, 1, on_write_end);
}

/**
 * tunnel socket connect
 * @param req
 * @param status
 */
void on_server_connect(uv_connect_t *req, int status) {
  // read data from req then send to client
  Tunnel* tunnel = (Tunnel *) req->data;
  assert(tunnel);
  printf("on_server_connect: tunnel address %p\n", tunnel);
  if (status < 0) {
    // 连接 server 失败
    fprintf(stderr, "%s\n", uv_strerror(status));
    free(tunnel);
    return;
  }
  printf("connect to real server correctly\n");
  tunnel->server = req->handle;
  if (tunnel->offset) {
    uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t uvBuf = uv_buf_init(tunnel->buf, tunnel->offset);
    tunnel->offset = 0;
    write_req->data = (void *) tunnel;
    uv_write(write_req, req->handle, &uvBuf, 1, on_write_end);
  }
  tunnel->server->data = (void *) tunnel;
  uv_read_start(tunnel->server, alloc_buffer, on_server_read);
}

void on_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  Tunnel *tunnel = (Tunnel *) stream->data;
  assert(tunnel);
  if (nread < 0) { // 客户端主动关闭连接
    if (nread != UV_EOF) {
      fprintf(stderr, "Error on reading client stream: %s.\n",
              uv_strerror(nread));
    }
    // close the connection
    uv_close((uv_handle_t *) stream, NULL);
    tunnel->client = NULL;
    printf("client closed\n");
  } else {
    // printf("client_stream address %p\n", stream);
    assert(stream == tunnel->client);

    // socks4a client data
    // parse destination address
    // TODO: validate socks client auth information
    // socks4a
    if (buf->base[0] == 0x04 && buf->base[1] == 0x01) {
      uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
      write_req->data = (void *) tunnel;
      char sendBuf[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};
      uv_buf_t uvBuf = uv_buf_init(sendBuf, sizeof(sendBuf));
      uv_write(write_req, stream, &uvBuf, 1, on_write_end);

      Addr *addr = parseAddr(buf->base, 2);
      tunnel->addr = addr;
      printf("server addr is: %s:%d\n", addr->ip, addr->port);
      const char *username = (char *)(buf->base + 8);
      if (strlen(username) > 0) {
        printf("username is: %s\n", username);
      }
      uv_tcp_t *socket = (uv_tcp_t* ) malloc(sizeof(uv_tcp_t));
      uv_tcp_init(loop, socket);
      uv_connect_t* connect = (uv_connect_t* )malloc(sizeof(uv_connect_t));
      connect->data = (void *) tunnel;
      struct sockaddr_in dest;
      uv_ip4_addr(addr->ip, addr->port, &dest);
      uv_tcp_connect(connect, socket, (const struct sockaddr *) &dest, on_server_connect);
    } else {
      memcpy(tunnel->header, buf->base, nread);
      tunnel->header[nread] = 0;
      if (tunnel->server == NULL) {
        printf("store request to buf is %s", buf->base);
        printf("tunnel->server doesn't exist, data size: %ld\n", nread);
        if (tunnel->offset + nread > MAXSIZE) {
          printf("tunnel has not enough buffer, server connection cannot created \n");
          return;
        }
        memcpy(tunnel->buf + tunnel->offset, buf->base, nread);
        tunnel->offset += nread;
        tunnel->buf[tunnel->offset] = 0;
      } else {
        uv_write_t* write_req = (uv_write_t* )malloc(sizeof(uv_write_t));
        write_req->data = (void *) tunnel;
        // write data to server
        printf("write data to tunnel server, server address: %s:%d\n", tunnel->addr->ip, tunnel->addr->port);
        printf("directly request is %s", buf->base);

        // Note: 必须要用新的 buf
        uv_buf_t tmpBuf = uv_buf_init(buf->base, nread);
        // printf("tunnel->server is %p\n", tunnel->server);
        assert(0 == uv_write(write_req, tunnel->server, &tmpBuf, 1, on_write_end));
        // assert(0 == uv_write(write_req, tunnel->server, buf, 1, on_write_end));
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
  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);

  // accept
  if (!uv_accept(server, (uv_stream_t *)client)) {
    Tunnel* tunnel = (Tunnel *)malloc(sizeof(Tunnel));
    tunnel->client = (uv_stream_t *) client;
    // Note: important
    tunnel->server = NULL;
    tunnel->status = TUNNEL_START;
    tunnel->offset = 0;
    printf("create tunnel success: %p\n", tunnel);
    printf("client stream is %p\n", client);
    client->data = (void *) tunnel;
    int r = uv_read_start((uv_stream_s *)client, alloc_buffer, on_client_read);
    if (r) {
      std::cout << "read the client error: " << uv_strerror(r)<< std::endl;
    } else {
      std::cout << "connect successfully" << std::endl;
    }
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
