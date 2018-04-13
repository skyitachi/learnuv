#include <uv.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <assert.h>

#define MAXSIZE 4096
#define MAXCONNECTION 128

#define CONTAINER_OF(ptr, type, field)                                        \
  ((type *) ((char *) (ptr) - ((char *) &((type *) 0)->field)))

char globalBuf[MAXCONNECTION][MAXSIZE];

uv_loop_t *loop;

int little_endian;
static int counter = 0;

int isLittleEndian() {
  short x = 0x0102;
  char *s = (char *) &x;
  return s[0] == 0x02 && s[1] == 0x01;
}

typedef struct Addr {
  char ip[16];
  int port;
} Addr;

typedef struct Tunnel {
  uv_stream_t* client; // stream between client and proxy
  uv_stream_t* server; // stream between server and proxy
  char buf[MAXSIZE];
  int offset;
  Addr* addr;
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
  *buf = uv_buf_init((char*) malloc(MAXSIZE), suggested_size);
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
  printf("in the uv_write\n");
}

/**
 * read data from server connection
 * @param stream
 * @param nread
 * @param buf
 */
void on_server_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Error on reading relay stream: %s.\n",
              uv_strerror(nread));
    }
    // close the connection
    // uv_close((uv_handle_t *) stream, NULL);
    // std::cout << "real server closed" << std::endl;
    return;
  }
  printf("server stream %x\n", (char *) stream);
  printf("read %ld size data from the server\n", nread);
  // Note
  Tunnel *tunnel = (Tunnel *) stream->data;
  assert(tunnel);
  uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
  uv_write(write_req, tunnel->client, buf, 1, on_write_end);
}

/**
 * tunnel socket connect
 * @param req
 * @param status
 */
void on_server_connect(uv_connect_t *req, int status) {
  // read data from req then send to client
  if (status < 0) {
    fprintf(stderr, "%s\n", uv_strerror(status));
    return;
  }
  printf("connect to real server correctly\n");
  Tunnel* tunnel = (Tunnel *) req->data;
  assert(tunnel);
  printf("on_server_connect: tunnel address %x\n", tunnel);
  tunnel->server = req->handle;
  if (tunnel->offset) {
    uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t uvBuf = uv_buf_init(tunnel->buf, tunnel->offset);
    tunnel->offset = 0;
    uv_write(write_req, req->handle, &uvBuf, 1, on_write_end);
  }
  tunnel->server->data = (void *) tunnel;
  uv_read_start(tunnel->server, alloc_buffer, on_server_read);
}

void on_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Error on reading client stream: %s.\n",
              uv_strerror(nread));
    }
    // close the connection
    // uv_close((uv_handle_t *) stream, NULL);
    // std::cout << "client closed" << std::endl;
  } else {
    printf("client_stream address %x\n", (char *)stream);
    Tunnel *tunnel = (Tunnel *) stream->data;
    assert(tunnel);
    printf("stream address %x, tunnel->client address %x\n", stream, tunnel->client);
    assert(stream == tunnel->client);

    // socks4a client data
    // parse destination address
    // TODO: validate socks client auth information
    // socks4a
    if (buf->base[0] == 0x04 && buf->base[1] == 0x01) {
      uv_write_t* write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
      char sendBuf[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};
      uv_buf_t uvBuf = uv_buf_init(sendBuf, sizeof(sendBuf));
      uv_write(write_req, stream, &uvBuf, 1, on_write_end);

      Addr *addr = parseAddr(buf->base, 2);
      tunnel->addr = addr;
      printf("server addr is: %s:%d\n", addr->ip, addr->port);

      uv_tcp_t *socket = (uv_tcp_t* ) malloc(sizeof(uv_tcp_t));
      uv_tcp_init(loop, socket);
      uv_connect_t* connect = (uv_connect_t* )malloc(sizeof(uv_connect_t));
      connect->data = stream->data;
      struct sockaddr_in dest;
      uv_ip4_addr(addr->ip, addr->port, &dest);
      uv_tcp_connect(connect, socket, (const struct sockaddr *) &dest, on_server_connect);
    } else {
      // Note: read real data
      printf("read the content: %d\n", nread);
      if (tunnel->server == NULL) {
        printf("tunnel->server doesn't exist, data size: %ld\n", nread);
        if (tunnel->offset + nread > MAXSIZE) {
          printf("tunnel has not enough buffer, server connection cannot created \n");
          return;
        }
        memcpy(tunnel->buf + tunnel->offset, buf->base, nread);
        tunnel->offset += nread;
      } else {
        uv_write_t* write_req = (uv_write_t* )malloc(sizeof(uv_write_t));
        // write data to server
        printf("write data to tunnel server, server address: %s:%d\n", tunnel->addr->ip, tunnel->addr->port);
        printf("tunnel->server is %x\n", tunnel->server);
        assert(0 == uv_write(write_req, tunnel->server, buf, 1, on_write_end));
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
    tunnel->offset = 0;
    printf("create tunnel success: %x\n", tunnel);
    printf("client stream is %x\n", (char *)client);
    client->data = (void *) tunnel;
    int r = uv_read_start((uv_stream_s *)client, alloc_buffer, on_client_read);
    if (r) {
      std::cout << "read the client error: " << uv_strerror(r)<< std::endl;
    } else {
      std::cout << "connect successfully" << std::endl;
    }
  }
}

int main() {
  little_endian = isLittleEndian();
  if (little_endian) {
    printf("little endian\n");
  } else {
    printf("big endian\n");
  }
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  printf("server stream address is %x\n", (char *) &server);
  sockaddr_in addr;
  uv_ip4_addr("127.0.0.1", 3000, &addr);

  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0); // bind

  int r = uv_listen((uv_stream_t *)&server, 2, on_new_client_connection); // listen
  if (r) {
    std::cout << "listen error: " << uv_strerror(r) << std::endl;
  } else {
    std::cout << "listen successfully" << std::endl;
  }

  return uv_run(loop, UV_RUN_DEFAULT);
}