package main

import (
	"bytes"
	"fmt"
	"github.com/satori/go.uuid"
	"strings"
)

const (
	variable = "var"
	constant = "const"
	FUNCVAL  = "func"
)

type Val struct {
	Type string
	v    interface{}
}

type FuncVal struct {
	FuncName string
	CallArgs []string
	RvArgs   Val

	PrevFuncVal *FuncVal
	NextFuncVal *FuncVal
}

var valMap map[string]string
var funcMap map[string]func(v *FuncVal) error

func init() {
	valMap = make(map[string]string, 10)
	valMap["parent_url"] = "http://127.0.0.1"
	valMap["base_url"] = "127.0.0.1"
	valMap["http_body"] = `{"A":"B"}`

	funcMap = make(map[string]func(v *FuncVal) error, 10)

	funcMap["uuid"] = GenUUID
}

func lookFunc(funcName string) bool {
	_, ok := funcMap[funcName]
	return ok
}

func callFunc(val *FuncVal) (*Val, bool) {
	cb, ok := funcMap[val.FuncName]
	if !ok {
		return nil, ok
	}

	err := cb(val)
	if err != nil {
		return nil, false
	}

	return &val.RvArgs, true
}

func findVal(s string) (string, bool) {
	v, ok := valMap[s]
	return v, ok
}

func GenUUID(v *FuncVal) error {
	u1 := uuid.Must(uuid.NewV4())
	v.RvArgs = Val{Type: variable, v: u1.String()}

	return nil
}

func parseFunc(s string, i *int) *FuncVal {

	j := *i
	start := j

	funcName := ""
	arg := ""
	var val *FuncVal

	for j < len(s) {
		if funcName == "" {

			if s[j] == ' ' || s[j] == '(' {

				funcName = strings.TrimSpace(s[start:j])

				ok := lookFunc(funcName)
				if !ok {
					panic("not find function " + funcName + "()")
				}

				val = &FuncVal{FuncName: funcName}

				if s[j] == '(' {
					j++
					start = j
				}
				goto args
			}

			goto next
		}

	args:
		if s[j] == ' ' {
			goto next
		}

		if s[j] == '(' {
			goto next
		}

		if s[j] == ',' {
			arg = strings.TrimSpace(s[start:j])
			j++
			start = j
		}

		if s[j] == ')' {
			arg = strings.TrimSpace(s[start:j])
			if len(arg) > 0 {
				val.CallArgs = append(val.CallArgs, arg)
			}

			*i = j

			return val
		}
	next:
		j++
	}

	return nil
}

func Parse(s []byte) []byte {

	var token bytes.Buffer

	var vals []Val

	var out bytes.Buffer

	pos := bytes.Index(s, []byte("$"))

	if pos == -1 {
		return s
	}

	isVariable := false
	isFunc := false

	for i := 0; i < len(s); {

		v := s[i]
		if v == '$' {

			if token.Len() > 0 {
				vals = append(vals, Val{Type: constant, v: token.String()})
				token.Reset()
			}

			if i+1 < len(s) && s[i+1] == '$' {
				i += 2 //skip $$
				isFunc = true

				goto callFunc
			}

			isVariable = true
		}

	callFunc:
		if isFunc {
			funcVal := parseFunc(string(s), &i)
			v, ok := callFunc(funcVal)
			if !ok {
				panic("call func " + funcVal.FuncName)
			}

			vals = append(vals, *v)
			isFunc = false
			goto next
		}

		if isVariable {

			vv, ok := findVal(token.String())
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

			if len(vals) > 0 && vals[len(vals)-1].Type == "+" {
				vals = vals[:len(vals)-1]
			}

			isVariable = false
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
				if !(len(vals) > 0 && vals[len(vals)-1].Type == variable) {
					vals = append(vals, Val{Type: "+", v: string(s[i:j])})
				}
				i = j
				v = s[i]
			}

			if s[i] == '$' {
				goto now
			}
			isVariable = false

		}

		token.WriteByte(v)

	next:
		i++
	now:
	}

	//fmt.Printf("--->%s\n", token.String())
	if token.Len() > 0 {
		vals = append(vals, Val{Type: constant, v: token.String()})
	}

	//fmt.Printf("%s\n", vals)

	for _, v := range vals {
		out.WriteString(v.v.(string))
	}

	fmt.Printf("%s\n", out.String())

	return out.Bytes()
}

func main() {

	ss := []string{
		//"./error.log $http_code != 200",
		"$parent_url + /eval/mp3",
		"http:// + $base_url + /eval/mp3",
		"$parent_url+/eval/mp3",
		"$parent_url+ /eval/mp3",
		"http:// + $base_url + /eval/mp3",
		"session-id:$$uuid()+:sh",
		"$$uuid()-area+bb",
		//"<$$uuid()> $http_body",
	}

	for _, v := range ss {
		Parse([]byte(v))
	}
}
