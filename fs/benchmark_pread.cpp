//
// Created by skyitachi on 2019-01-06.
//

#include <unistd.h>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>

// 4k 读多文件
int main() {
  int partitionNum = 1024;
  char buffer[4096];
  auto start = std::chrono::system_clock::now();
  for(int i = 0; i < partitionNum; i++) {
    std::string filename = std::string("/tmp/test_engine3") + "/VALUE" + std::to_string(i);
    int fd = open(filename.c_str(), O_RDWR, 0644);
    struct stat st;
    fstat(fd, &st);
    int64_t len = st.st_size;
//    for(int64_t j = len - 4096; j >= 0; j -= 4096) {
//      pread(fd, buffer, 4096, j);
//    }
    for (int64_t j = 0; j < len; j += 4096) {
      pread(fd, buffer, 4096, j);
    }
  }
  
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> duration = end - start;
  std::cout << duration.count() << std::endl;
  
}

