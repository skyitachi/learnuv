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

const BUF_SIZE = 1 << 16

func main() {
  var totalWrites int64 = 0
  if len(os.Args) != 6 {
    fmt.Fprintln(os.Stderr, "Usage: go run client.go <host> <port> <sessions> <blocksize> <timeout>")
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
  timeout, err := strconv.Atoi(os.Args[5])
  if err != nil {
    log.Fatal(err)
  }
  for i := 0; i < blocksize; i++ {
    bufs = append(bufs, byte(i % 128))
  }
  var wg sync.WaitGroup
  for i := 0; i < sessions; i++ {
    wg.Add(1)
    go func() {
      conn, err := net.Dial("tcp", host + ":" + port)
      if err != nil {
        log.Println(err)
      }
      go func() {
        wc, err:= io.Copy(conn, bytes.NewReader(bufs))
        if err != nil {
          log.Println(err)
        }
        if wc != int64(len(bufs)) {
          log.Println("cannot write enough data")
        }
        totalWrites += wc
      }()
      go func() {
        var rbuf =  make([]byte, BUF_SIZE)
        for {
          rc, err := conn.Read(rbuf)
          if err != nil {
            // 服务端不会主动关闭连接
            break
          }
          wc, err := conn.Write(rbuf[:rc])
          if err != nil {
            log.Println("connection write error ", err)
            break
          }
          totalWrites += int64(wc)
        }
      }()
      t := time.NewTimer(time.Duration(timeout) * time.Second)
      for {
        select {
          case _ = <-t.C:
            conn.Close()
            fmt.Println("connection close")
            wg.Done()
        }
      }
    }()
  }
  wg.Wait()
  fmt.Printf("total rate is %f MB/s\n", (float64(totalWrites) * 1.0) / float64(1024 * 1024 * timeout))
}
