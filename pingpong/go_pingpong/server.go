package main

import (
  "os"
  "fmt"
  "net"
  "io"
  "log"
)

const BUF_SIZE = 1 << 16

func accept(server net.Listener) {
  go func() {
    for {
      conn, err := server.Accept()
      if err != nil {
        log.Println(err.Error())
      } else {
        go connect(conn)
      }
    }
  }()
}

func connect(conn net.Conn) {
  fmt.Println("connection comes")
  var buf = make([]byte, BUF_SIZE)
  count := 0
  for {
    c, err := conn.Read(buf)
    count += c
    if err != nil {
      if err == io.EOF {
        log.Printf("read %d size data", count)
        break
      } else {
        log.Println(err)
      }
    }
    if c == 0 {
      log.Println("read count is: 0 from ", conn.RemoteAddr())
      break
    }
    c2, err := conn.Write(buf[:c])
    if err != nil {
      log.Println("Connection Write Error: ", err)
      continue
    }
    if c != c2 {
      log.Println("Read and Write doesn't equal")
    }
  }
}

func main() {
  done := make(chan int)
  if len(os.Args) != 3 {
    fmt.Fprint(os.Stderr, "Usage: go run server.go <host> <port>")
    os.Exit(1)
  }
  host := os.Args[1]
  port := os.Args[2]
  server, err := net.Listen("tcp", host + ":" + port)
  if err != nil {
    fmt.Fprint(os.Stderr, err.Error())
    os.Exit(1)
  }
  go accept(server)
  <-done
}
