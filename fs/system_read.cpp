//
// Created by skyitachi on 2018/6/3.
//

#include <stdio.h>
#include <uv.h>
#include <cstdlib>
#include <unistd.h>
#include <math.h>

#define BUF_SIZE 4096

int main(int argc, char **argv) {
  int buf_size = BUF_SIZE;
  if (argc < 2 || argc > 3) {
    printf("Usage: ./read $file $buf_size\n");
    exit(1);
  }
  if (argc == 3) {
    buf_size = atoi(argv[2]);
  }

  char *buf = (char *) malloc(buf_size);
  uint64_t start = uv_hrtime();
  int fd = open(argv[1], O_RDONLY, 0);

  if (fd < 0) {
    printf("open file error\n");
  }
  int count = 0;
  while(true) {
    int n = read(fd, buf, buf_size);
    if (n <= 0) {
      break;
    }
    count += n;
  }
  printf("total file size: %d\n", count);
  printf("time consume: %lf\n", floor((uv_hrtime() - start) * 1.0 / 1e6));
  close(fd);
}

