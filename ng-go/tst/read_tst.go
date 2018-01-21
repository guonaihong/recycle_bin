package main

import (
	"fmt"
	"log"
	"os"
)

//golang的bytes.Buffer数据结构不支持回退到过去读取的位置
//没有提够方法操作pos变量，pos -= n(n > 0)，其中的Next方法只能改变pos += n
type Buffer struct {
	Pos int
	Buf []byte
}

func NewBuffer(buf []byte) *Buffer { return &Buffer{Buf: buf} }

func (b *Buffer) Shrink(start int, l int) {
	copy(b.Buf, b.Buf[start:l])
}

func main() {

	b := NewBuffer(make([]byte, 4096))
	start := b.Pos

	fi, err := os.Stdin.Stat()
	if err != nil {
		fmt.Printf("stdint fail:%v\n", err)
		return
	}

	offset := 0
	fileSize := int(fi.Size())

	for {

		if b.Pos >= cap(b.Buf) || b.Pos == 0 {

			size := fileSize - offset
			if size == 0 {

				//log.Printf("bye bye\n")
				return
			}

			l := b.Pos - start

			if l > 0 {
				b.Shrink(start, l)
			}

			if size > cap(b.Buf)-l {
				size = cap(b.Buf) - l
			}

			//log.Printf("pos(%d):size(%d)\n", b.Pos, size)

			n, err := os.Stdin.Read(b.Buf[b.Pos:size])

			if err != nil {
				log.Printf("read fail:%v:pos(%d), size(%d)\n", err, b.Pos, size)
				return
			}

			//log.Printf("read file:%d:%d\n", n, size)
			if n != size {
				log.Printf("n != size\n")
				return
			}

			b.Pos = l
			start = 0
			offset += n

		}

		var ch byte
		//log.Printf("b.Pos(%d), len(%d), offset(%d)\n", b.Pos, len(b.Buf), offset)
		if b.Pos < offset {
			ch = b.Buf[b.Pos]
		}
		b.Pos++
		fmt.Printf("%c", ch)
	}
}
