package main

import (
	"bytes"
	"fmt"
	"io"
	"strings"
)

const bufSize = 4

func printOffset(rs io.ReadSeeker, w io.Writer, buf []byte, start, end int64) error {

	curPos, err := rs.Seek(0, 1)
	if err != nil {
		return err
	}

	_, err = rs.Seek(start, 0)
	if err != nil {
		return err
	}

	defer rs.Seek(curPos, 0)

	for {

		if start >= end {
			break
		}

		needRead := end - start
		if int(needRead) > len(buf) {
			needRead = int64(len(buf))
		}

		n, e := rs.Read(buf[:needRead])
		if e != nil {
			break
		}

		w.Write(buf[:n])
		start += int64(n)
	}
	return nil
}

func ReadFromTail(rs io.ReadSeeker, w io.Writer, sep []byte) error {

	tail, err := rs.Seek(0, 2)
	if err != nil {
		return err
	}

	head := tail

	buf := make([]byte, bufSize+len(sep))
	buf2 := make([]byte, bufSize)

	for head > 0 {

		minRead := head
		if minRead > bufSize {
			minRead = bufSize
		}

		_, err := rs.Seek(-minRead, 1)
		if err != nil {
			return err
		}

		n, err := rs.Read(buf[:minRead])
		if err != nil {
			return err
		}

		head -= minRead
		rs.Seek(-minRead, 1)

		t := n
		h := n

		for {
			pos := bytes.LastIndex(buf[:h], sep)

			if pos == -1 {
				//not found
				break
			}

			if pos >= 0 {

				w.Write(buf[pos+len(sep) : t])
				if l := t - pos - len(sep); l > 0 {
					tail -= int64(l)
				}

				if !bytes.Equal(buf[pos:t], sep) {
					t = pos - len(sep)
				}

				h = pos - 1

				if tail > head+minRead {
					err = printOffset(rs, w, buf2, head+minRead, tail)
					if err != nil {
						return err
					}
					tail = head + minRead
				}

				if pos == 0 {
					break
				}
			}

		}
	}

	if tail > 0 {
		printOffset(rs, w, buf2, 0, tail)
	}
	return nil
}

func main() {
	/*
		s := []string{
			"0123456789",
			"abcdefghij",
			"klmnopqrst",
		}

		fmt.Printf("%s\n", strings.Join(s, "123"))
		sr := strings.NewReader(strings.Join(s, "123"))
		ReadFromTail(sr, os.Stdout, []byte("123"))
	*/

	s := []string{
		"1\n",
		"2\n",
		"3\n",
		"4\n",
		"5\n",
	}

	w := &bytes.Buffer{}
	sr := strings.NewReader(strings.Join(s, ""))
	err := ReadFromTail(sr, w, []byte("\n"))
	if err != nil {
		fmt.Printf("%s\n", err)
	}

	fmt.Printf("out(%s)\n", w.String())
}
