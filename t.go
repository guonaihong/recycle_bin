package main

import (
	"bytes"
	"fmt"
	"strings"
)

const (
	variable = "var"
	constant = "const"
)

type Val struct {
	Type string
	v    interface{}
}

var valMap map[string]string

func parse(s []byte) {
	pos := bytes.Index(s, []byte("$"))

	var token bytes.Buffer

	var vals []Val

	var out bytes.Buffer

	if pos != -1 {

		findVal := false
		for i := 0; i < len(s); {

			v := s[i]
			if v == '$' {
				if token.Len() > 0 {
					vals = append(vals, Val{Type: constant, v: token.String()})
				}
				findVal = true
			}

			if findVal {

				vv, ok := valMap[token.String()]
				if !ok {
					if v == '$' {
						goto next
					}

					if v == ' ' || v == '+' {
						fmt.Printf("not found $%s\n", token.String())
						goto space
					}

					token.WriteByte(v)

					goto next
				}

				//fmt.Printf("find val:%s\n", token.String())
				if len(vals) > 0 && vals[len(vals)-1].Type == "+" {
					vals = vals[:len(vals)-1]
				}

				findVal = false
				vals = append(vals, Val{Type: variable, v: vv})
				token.Reset()
			}

		space:
			if v == ' ' || v == '+' {

				if token.Len() > 0 {
					//fmt.Printf("token(%s)\n", token.Bytes())
					vals = append(vals, Val{Type: constant, v: token.String()})
					token.Reset()
				}

				j := i
				for ; j < len(s); j++ {

					if s[j] == '+' {
						continue
					}

					if s[j] != ' ' {
						break
					}

				}

				if strings.TrimSpace(string(s[i:j])) == "+" {
					i = j
					v = s[i]
					if !(len(vals) > 0 && vals[len(vals)-1].Type == variable) {
						vals = append(vals, Val{Type: "+", v: token.String()})
					}
				}

				if s[i] == '$' {
					goto now
				}
				findVal = false

			}

			token.WriteByte(v)

		next:
			i++
		now:
		}
	}

	if token.Len() > 0 {
		vals = append(vals, Val{Type: constant, v: token.String()})
	}

	//fmt.Printf("%s\n", vals)

	for _, v := range vals {
		out.WriteString(v.v.(string))
	}

	fmt.Printf("%s\n", out.String())
}

func main() {

	valMap = make(map[string]string, 10)
	valMap["parent_url"] = "http://127.0.0.1"
	valMap["base_url"] = "127.0.0.1"
	valMap["http_body"] = `{"A":"B"}`

	ss := []string{
		//"<$$uuid()> $http_body",
		//"./error.log $http_code != 200",
		"$parent_url + /eval/mp3",
		"http:// + $base_url + /eval/mp3",
		"$parent_url+/eval/mp3",
		"$parent_url+ /eval/mp3",
		"http:// + $base_url + /eval/mp3",
	}

	for _, v := range ss {
		parse([]byte(v))
	}
}
