package main

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"strings"
)

const bufSize = 4

func ReadFromTail(rs io.ReadSeeker, wr io.Writer, sep []byte) {

	tail, _ := rs.Seek(0, 2)
	head := tail
	buf := make([]byte, bufSize+len(sep))

	for tail > 0 {

		minRead := tail
		if minRead > bufSize {
			minRead = bufSize
		}

		_, err := rs.Seek(-minRead, 1)
		if err != nil {
			break
		}

		_, err = rs.Read(buf[:len(buf)-len(sep)])
		if err != nil {
			break
		}

		head -= minRead
		rs.Seek(-minRead, 1)

		for {
			pos := bytes.LastIndex(buf, sep)
			w := len(buf)

			if pos == -1 {
				break
			}

			if pos == 0 {
				//todo
			}

			if pos > 0 {
				fmt.Printf("buf = %s\n", buf)
				wr.Write(buf[pos+len(sep) : w])
				w = pos - 1
				if pos == len(sep) {
					break
				}

				continue
			}

			if w > 0 {
				pos := bufSize
				if tail < bufSize {
					pos = int(tail)
				}

				copy(buf[pos:], buf[:w])
				tail -= int64(w)
				break
			}
		}
	}
}

func main() {
	s := []string{
		"0123456789",
		"abcdefghij",
		"klmnopqrst",
	}

	fmt.Printf("%s\n", strings.Join(s, "123"))
	sr := strings.NewReader(strings.Join(s, "123"))
	ReadFromTail(sr, os.Stdout, []byte("123"))
}
