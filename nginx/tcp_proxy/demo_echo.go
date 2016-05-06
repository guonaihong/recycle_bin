package main

import (
	"flag"
	"log"
	"net"
)

func worker(conn net.Conn) {
	defer func() {
		conn.Close()
		log.Printf("%p bye bye\n", &conn)
	}()

	buf := make([]byte, 512)
	for {
		rv, err := conn.Read(buf)
		if rv == 0 {
			return
		}

		if err != nil {
			log.Printf("%p recv fail:%v\n", err)
			return
		}

		log.Printf("(%s)\n", buf)
		rv, err = conn.Write(buf)
		if err != nil {
			log.Printf("%p send fail:%v\n", err)
		}
	}
}

func main() {
	sa := flag.String("sa", ":8000", "echo server ip:port")

	flag.Parse()

	l, err := net.Listen("tcp", *sa)

	if err != nil {
		log.Printf("echo server listen:%s\n", err.Error())
		return
	}
	defer l.Close()

	log.Printf("listen addr:%s\n", *sa)

	for {
		conn, err := l.Accept()
		if err != nil {
			log.Printf("accept %s\n", err)
			return
		}

		log.Printf("recv message %s-->%s\n", conn.RemoteAddr(), conn.LocalAddr())

		go worker(conn)
	}
}
