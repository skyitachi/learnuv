#include "util.h"

uv_loop_t* loop;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: server <address> <port>\n");
    return;
  }
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  
  sockadd_in addr;
  uv_ip4_addr();
  
}

