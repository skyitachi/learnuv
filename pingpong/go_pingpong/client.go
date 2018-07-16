package main

import (
  "os"
  "fmt"
  "strconv"
  "log"
  "sync"
  "net"
  "io"
  "bytes"
  "time"
)
var bufs []byte

func main() {
  if len(os.Args) != 5 {
    fmt.Fprintln(os.Stderr, "Usage: go run client.go <host> <port> <sessions> <blocksize>")
    os.Exit(1)
  }
  host := os.Args[1]
  port := os.Args[2]
  sessions, err := strconv.Atoi(os.Args[3])
  if err != nil {
    log.Fatal(err)
  }
  fmt.Println("sessions are ", sessions)
  blocksize, err := strconv.Atoi(os.Args[4])
  if err != nil {
    log.Fatal(err)
  }
  for i := 0; i < blocksize; i++ {
    bufs = append(bufs, byte(i % 128))
  }
  var wg sync.WaitGroup
  start := time.Now()
  for i := 0; i < sessions; i++ {
    wg.Add(1)
    go func() {
      conn, err := net.Dial("tcp", host + ":" + port)
      if err != nil {
        log.Println(err)
      }
      wc, err:= io.Copy(conn, bytes.NewReader(bufs))
      if err != nil {
        log.Println(err)
      }
      if wc != int64(len(bufs)) {
        log.Println("cannot write enough data")
      }
      wg.Done()
    }()
  }
  wg.Wait()
  ns := float64(time.Now().UnixNano() - start.UnixNano()) / 1e9
  fmt.Println("total time consumes: ", time.Now().Sub(start))
  fmt.Printf("total rate is %f MB/s\n", (float64(blocksize * sessions) * 1.0) / (1024 * 1024 * 1.0) / ns)
}
