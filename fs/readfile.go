package main

import (
  "bufio"
  "os"
  "time"
  "fmt"
  "io"
  "strconv"
)

func main() {
  argc := len(os.Args)
  if argc < 2 || argc > 3 {
    fmt.Println("Usage: go run readfile.go foo.txt 4096")
    return
  }
  buf_size := 4096
  if argc == 3 {
    if s, err := strconv.Atoi(os.Args[2]); err == nil {
      buf_size = s
    }
  }
  start := time.Now()
  var buf = make([]byte, buf_size)
  f, err := os.Open(os.Args[1])
  if err != nil {
    fmt.Println(err)
    return
  }
  r := bufio.NewReader(f)
  count := 0
  for {
    c, err := r.Read(buf)
    if err != nil {
      if err == io.EOF {
        break
      }
      fmt.Println(err)
      return
    }
    count += c
  }
  fmt.Println("total file size: ", count)

  fmt.Println("time consumes: ", time.Now().Sub(start))
}


