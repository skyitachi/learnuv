#ifndef DEFS_H_
#define DEFS_H_

#include <uv.h>

#define MAXSIZE 4096
#define REQUEST_SIZE 512

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
  CLIENT_WRITE_END,
  CONNECTING_TO_SERVER,
  CONNECTED_TO_SERVER,
} Status;

//typedef struct Tunnel {
//  int offset;
//  Addr* addr;
//  char buf[MAXSIZE];
//  char header[REQUEST_SIZE];
//  Status status;
//  uv_stream_t* client; // stream between client and proxy
//  uv_stream_t* server; // stream between server and proxy
//} Tunnel;

typedef struct Tunnel {
  Status status;
  Addr addr;
  // why union
  union {
    uv_handle_t handle;
    uv_stream_t stream;
    uv_tcp_t tcp;
  } client;
  union {
    uv_handle_t handle;
    uv_stream_t stream;
    uv_tcp_t tcp;
  } server;
  uv_stream_t server_stream;
  uv_connect_t connect_req;
  uv_write_t client_write_req;
  uv_write_t server_write_req;
  size_t server_buf_offset;
  char client_buf[MAXSIZE];
  char server_buf[MAXSIZE];
} Tunnel;

/* This macro looks complicated but it's not: it calculates the address
 * of the embedding struct through the address of the embedded struct.
 * In other words, if struct A embeds struct B, then we can obtain
 * the address of A by taking the address of B and subtracting the
 * field offset of B in A.
 */
#define CONTAINER_OF(ptr, type, field)                                        \
  ((type *) ((char *) (ptr) - ((char *) &((type *) 0)->field)))

#endif
