package main

import (
	"bytes"
	"fmt"
	"strings"
)

var val map[string]string

func parse(s []byte) {
	pos := bytes.Index(s, []byte("$"))

	var out bytes.Buffer
	var token bytes.Buffer

	if pos != -1 {

		findVal := false
		for i := 0; i < len(s); {

			v := s[i]
			if v == '$' {
				findVal = true
			}

			if findVal {

				vv, ok := val[token.String()]
				if !ok {
					if v == '$' {
						goto next
					}
					token.WriteByte(v)
					goto next
				}

				findVal = false
				out.WriteString(vv)
				token.Reset()

				j := i
				for ; j < len(s); j++ {

					if s[j] == '+' {
						s[j] = ' '
						continue
					}

					if s[j] != ' ' {
						break
					}

				}

				if strings.TrimSpace(string(s[i:j])) == "" {
					i = j - 1
					goto next
				}
			}

			if v == ' ' {
				findVal = false
				out.Write(token.Bytes())
			}

			out.WriteByte(v)

		next:
			i++
		}
	}

	fmt.Printf("%s\n", out.Bytes())
}

func main() {

	val = make(map[string]string, 10)
	val["parent_url"] = "http://127.0.0.1"
	val["http_body"] = `{"A":"B"}`

	ss := []string{
		//"<$$uuid()> $http_body",
		//"./error.log $http_code != 200",
		"$parent_url + /eval/mp3",
	}

	for _, v := range ss {
		parse([]byte(v))
	}
}
