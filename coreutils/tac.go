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

	total, _ := rs.Seek(0, 2)
	buf := make([]byte, bufSize+len(sep))

	for total > 0 {
		minRead := total
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

		rs.Seek(-minRead, 1)

		for {
			// if not found, continue backtracking
			// todo
			r := bytes.LastIndex(buf, sep)
			w := len(buf)

			if r == -1 {
				wr.Write(buf[0:w])
				//break
			}

			if r > 0 {
				fmt.Printf("buf = %s\n", buf)
				wr.Write(buf[r+len(sep) : w])
				w = r - 1
				if r == len(sep) {
					break
				}

				continue
			}

			if w > 0 {
				r := bufSize
				if total < bufSize {
					r = int(total)
				}

				copy(buf[r:], buf[:w])
				total -= int64(w)
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
