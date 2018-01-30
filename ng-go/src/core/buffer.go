package core

//golang的bytes.Buffer数据结构不支持回退到过去读取的位置
//没有提够方法操作pos变量，pos -= n(n > 0)，其中的Next方法只能改变pos += n
type Buffer struct {
	Pos     int
	Buf     []byte
	Start   int
	End     int
	Last    int
	Flush   bool
	LastBuf bool
}

type Chain struct {
	Buf  Buffer
	Next *Chain
}

func NewBuffer(buf []byte) *Buffer { return &Buffer{Buf: buf} }

func (b *Buffer) Shrink(start int, l int) {
	copy(b.Buf, b.Buf[start:l])
}
