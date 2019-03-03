package main

import (
	"bufio"
	"fmt"
	"os"
	"unicode"
	"unicode/utf8"
)

type Results struct {
	Lines     int64
	Words     int64
	Chars     int64
	Bytes     int64
	MaxLength int64
}

type Counter struct {
	TabWidth int64
}

func NewCounter() *Counter {
	return &Counter{TabWidth: 8}
}

func (c *Counter) countComplicated() (res Results, err error) {
	var (
		pos    int64
		inWord bool
	)

	br := bufio.NewReader(os.Stdin)

	for {
		l, e := br.ReadBytes('\n')
		if e != nil && len(l) == 0 {
			break
		}

		for bp := 0; bp < len(l); {
			r, s := utf8.DecodeRune(l[bp:])
			if r >= utf8.RuneSelf {
				//bp += s
				//continue
			}

			switch r {
			case '\n':
				res.Lines++
				fallthrough
			case '\r', '\f':
				if pos > res.MaxLength {
					res.MaxLength = pos
				}
				pos = 0
				if inWord {
					res.Words++
				}
				inWord = false
			case '\t':
				pos += c.TabWidth - (pos % c.TabWidth)
				if inWord {
					res.Words++
				}
				inWord = false
			case ' ':
				pos++
				fallthrough
			case '\v':
				if inWord {
					res.Words++
				}
				inWord = false
			default:
				if unicode.IsPrint(r) {

					pos++
					if unicode.IsSpace(r) {
						if inWord {
							res.Words++
						}
						inWord = false
					} else {
						inWord = true
					}
				}
			}
			res.Chars++
			bp += s
		}
	}

	if pos > res.MaxLength {
		res.MaxLength = pos
	}
	if inWord {
		res.Words++
	}
	return res, nil
}

func main() {
	c := NewCounter()
	r, err := c.countComplicated()
	if err != nil {
		panic(err.Error())
	}
	fmt.Printf("%#v\n", r)
}
