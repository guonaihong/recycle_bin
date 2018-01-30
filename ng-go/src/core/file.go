package core

import (
	"errors"
	"io"
	"log"
	"os"
	"reflect"
	"strings"
	_ "unsafe"
)

const PtrSize = 4 << (^uintptr(0) >> 63)

type Conf struct {
	Cycle      Cycle
	Name       string
	Args       []string
	ModuleType uint
	CmdType    uint
	ConfFile   ConfFile
	Ctx        interface{}
}

type ConfFile struct {
	File   *os.File
	Line   uint
	Offset int
	Buf    *Buffer
}

const (
	BLOCK_START = 1
	BLOCK_DONE  = 2
	FILE_DONE   = 3
)

const (
	MAIN_CONF = 0x01000000
	ANY_CONF  = 0x1F000000
)

const (
	CONF_NOARGS = 0x00000001
	CONF_TAKE1  = 0x00000002
	CONF_TAKE2  = 0x00000004
	CONF_TAKE3  = 0x00000008
	CONF_TAKE4  = 0x00000010
	CONF_TAKE5  = 0x00000020
	CONF_TAKE6  = 0x00000040
	CONF_TAKE7  = 0x00000080
)

const (
	CONF_TAKE12   = (CONF_TAKE1 | CONF_TAKE2)
	CONF_TAKE13   = (CONF_TAKE1 | CONF_TAKE3)
	CONF_TAKE23   = (CONF_TAKE2 | CONF_TAKE3)
	CONF_TAKE123  = (CONF_TAKE1 | CONF_TAKE2 | CONF_TAKE3)
	CONF_TAKE1234 = (CONF_TAKE1 | CONF_TAKE2 | CONF_TAKE3 | CONF_TAKE4)
)
const (
	CONF_MAX_ARGS = 8
)

const (
	CONF_ARGS_NUMBER = 0x000000ff
	CONF_BLOCK       = 0x00000100
	CONF_FLAG        = 0x00000200
	CONF_ANY         = 0x00000400
	CONF_1MORE       = 0x00000800
	CONF_2MORE       = 0x00001000
)

var ARGUMENT_NUMBER = []uint{
	CONF_NOARGS,
	CONF_TAKE1,
	CONF_TAKE2,
	CONF_TAKE3,
	CONF_TAKE4,
	CONF_TAKE5,
	CONF_TAKE6,
	CONF_TAKE7,
}

var Commands = []Command{
	{
		Name:   "string",
		Type:   ANY_CONF | CONF_TAKE1,
		Set:    Include,
		Conf:   0,
		Offset: 0,
		Post:   nil,
	},
}

func Include(cf *Conf, cmd *Command, conf interface{}) (int, error) {
	value := cf.Args
	file := value[1]

	if strings.IndexAny(cf.Name, "*?[") == -1 {
		return ConfParse(cf, file)
	}

	//TODO:
	return OK, nil
}

func readToken(cf *Conf) int {

	ch := byte(0)

	found := false
	needSpace := false
	lastSpace := true
	sharpComment := false

	variable := false
	quoted := false
	sQuoted := false
	dQuoted := false

	b := cf.ConfFile.Buf
	start := b.Pos //start = b.pos
	n := 0

	//log.Printf("b.pos = %d\n", b.Pos)
	var err error
	cf.Args = cf.Args[0:0]

	fi, err := cf.ConfFile.File.Stat()
	if err != nil {
		return ERROR
	}

	fileSize := int(fi.Size())

	for {

		if b.Pos >= cap(b.Buf) || b.Pos == 0 {
			size := fileSize - cf.ConfFile.Offset

			if size == 0 {

				return FILE_DONE
			}

			l := b.Pos - start

			if l > 0 {
				b.Shrink(start, l)
			}

			if size > cap(b.Buf)-l {
				size = cap(b.Buf) - l
			}
			n, err = cf.ConfFile.File.Read(b.Buf[b.Pos:size])

			if err != nil {
				if err == io.EOF {
					//TODO check
					return OK
				}
				return ERROR
			}

			if n != size {
				//TODO
				log.Printf("n != size")
				return ERROR
			}

			b.Pos = l
			start = 0
			cf.ConfFile.Offset += n
		}

		if b.Pos < cf.ConfFile.Offset {
			ch = b.Buf[b.Pos]
		}
		b.Pos++

		if ch == '\n' {

			cf.ConfFile.Line++

			if sharpComment {
				sharpComment = false
			}
		}

		if sharpComment {
			continue
		}

		if quoted {
			quoted = false
			continue
		}

		if needSpace {
			if ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' {
				lastSpace = true
				needSpace = false
				continue
			}

			if ch == ';' {
				return OK
			}

			if ch == '{' {
				return BLOCK_START
			}

			if ch == ')' {
				lastSpace = true
				needSpace = false

			} else {
				//TODO printf log
				return ERROR
			}

		}

		if lastSpace {
			if ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' {
				continue
			}

			start = b.Pos - 1
			//startLine = cf.ConfFile.Line;

			switch ch {

			case ';':
			case '{':
				if len(cf.Args) == 0 {
					log.Printf("unexpected \"%c\"", ch)
					return ERROR
				}

				if ch == '{' {
					return BLOCK_START
				}

				return OK

			case '}':
				if len(cf.Args) != 0 {
					log.Printf("unexpected \"}\"")
					return ERROR
				}

				return BLOCK_DONE

			case '#':
				sharpComment = true
				continue

			case '\\':
				quoted = true
				lastSpace = false
				continue

			case '"':
				start++
				dQuoted = true
				lastSpace = false
				continue

			case '\'':
				start++
				sQuoted = true
				lastSpace = false
				continue

			default:
				lastSpace = false
			}

		} else {
			if ch == '{' && variable {
				continue
			}

			variable = false

			if ch == '\\' {
				quoted = true
				continue
			}

			if ch == '$' {
				variable = true
				continue
			}

			if dQuoted {
				if ch == '"' {
					dQuoted = false
					needSpace = true
					found = true
				}

			} else if sQuoted {
				if ch == '\'' {
					sQuoted = false
					needSpace = true
					found = true
				}

			} else if ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == ';' || ch == '{' {
				lastSpace = true
				found = true
			}

			if found {

				word := make([]byte, b.Pos-1-start+1)

				if word == nil {
					return ERROR
				}

				length := 0
				for dst, src := 0, start; src < b.Pos-1; length++ {

					if b.Buf[src] == '\\' {
						switch b.Buf[src+1] {
						case '"':
						case '\'':
						case '\\':
							src++
							//break

						case 't':
							word[dst] = '\t'
							dst++
							src += 2
							continue

						case 'r':
							word[dst] = '\r'
							dst++
							src += 2
							continue

						case 'n':
							word[dst] = '\n'
							dst++
							src += 2
							continue
						}

					}
					//log.Printf("dst(%d), src(%d)\n", dst, src)
					word[dst] = b.Buf[src]
					dst++
					src++
				}

				log.Printf("word(%s), current args(%s)\n", word[:length], cf.Args)
				//log.Printf("length(%d), word = %s:%s\n", length, word, word[:length])
				word = word[:length]
				cf.Args = append(cf.Args, string(word))

				if ch == ';' {
					return OK
				}

				if ch == '{' {
					return BLOCK_START
				}

				found = false
			}
		}
	}
}

func ConfParse(cf *Conf, filename string) (int, error) {

	const (
		PARSE_FILE  = 0
		PARSE_BLOCK = 1
		PARSE_PARAM = 2
	)

	typeVal := 0

	var prev *ConfFile

	if filename != "" {
		fd, err := os.Open(filename)
		if err != nil {
			return ERROR, err
		}

		prev = &cf.ConfFile

		cf.ConfFile = ConfFile{}

		cf.ConfFile.File = fd
		cf.ConfFile.Buf = NewBuffer(make([]byte, 4096))
	} else if cf.ConfFile.File != nil {
		typeVal = PARSE_BLOCK
	} else {
		typeVal = PARSE_PARAM
	}

	rc := 0

	for {
		rc = readToken(cf)
		if rc == ERROR {
			goto done
		}

		if rc == BLOCK_DONE {

			if typeVal != PARSE_BLOCK {
				//todo printf errror
				goto failed
			}

			goto done
		}

		if rc == FILE_DONE {
			if typeVal == PARSE_BLOCK {
				goto failed
			}
			goto done
		}

		if rc == BLOCK_START {
			if typeVal == PARSE_PARAM {
				goto failed
			}
		}

		//log.Printf("#######:%v\n", cf.Cycle.Plugins)
		rc = Handler(cf, rc)
		if rc == ERROR {
			goto failed
		}
		log.Printf("args(%s)\n", cf.Args)
	}

failed:
	rc = ERROR
done:
	if filename != "" {

		cf.ConfFile = *prev
	}

	if rc == ERROR {
		log.Printf("ERROR:parse fail\n") //TODO
		return ERROR, errors.New("parse fail")
	}
	return 0, nil
}

func Handler(cf *Conf, last int) int {

	name := cf.Args[0]

	var cmd *Command
	var conf interface{}
	var rv int
	var pos int
	var err error
	var modules []*Module

	cmd, pos = cf.Cycle.GetCommand(name, cf.CmdType)

	if cmd == nil {
		log.Printf("unknown directive \"%s\"", name)
		return ERROR
	}
	log.Printf("cf.CmdType =======%p, %x, cmd(%v)\n", cf, cf.CmdType, cmd)

	if (cmd.Type&CONF_BLOCK) != CONF_BLOCK && last != OK {
		log.Printf("directive \"%s\" is not terminated by \";\"",
			name)
		return ERROR
	}

	if (cmd.Type&CONF_BLOCK) == CONF_BLOCK && last != BLOCK_START {
		log.Printf("directive \"%s\" has no opening \"{\"", name)
		return ERROR
	}

	if (cmd.Type & CONF_ANY) != CONF_ANY {

		if (cmd.Type & CONF_FLAG) == CONF_FLAG {

			if len(cf.Args) != 2 {
				goto invalid
			}

		} else if (cmd.Type & CONF_1MORE) == CONF_1MORE {

			if len(cf.Args) < 2 {
				goto invalid
			}

		} else if (cmd.Type & CONF_2MORE) == CONF_2MORE {

			if len(cf.Args) < 3 {
				goto invalid
			}

		} else if len(cf.Args) > CONF_MAX_ARGS {

			goto invalid

		} else if (cmd.Type & ARGUMENT_NUMBER[len(cf.Args)-1]) == 0 {
			goto invalid
		}
	}

	if cmd.Type&MAIN_CONF > 0 {
		modules = cf.Cycle.GetMainModules2(pos)
		conf = &modules[cmd.CtxIndex].Ctx
	} else if cf.Ctx != nil {
		//log.Printf("Conf(%d) cmd.Name(%s)\n", cmd.Conf, cmd.Name)
		val := reflect.ValueOf(cf.Ctx)

		if val.Kind() == reflect.Ptr {
			val = val.Elem()
		}

		tmp := val.Field(int(cmd.Conf)).Interface()
		confs := tmp.([]interface{})
		if confs[cmd.CtxIndex] != nil {
			conf = confs[cmd.CtxIndex]
		}
	}

	rv, err = cmd.Set(cf, cmd, conf)
	if rv == OK {
		return OK
	}

	if rv == ERROR {
		log.Printf("ERROR:cmd set fail:%s\n", err)
		return ERROR
	}

	log.Printf("\"%s\" directive (%d):(%s)", name, rv, err)

	return ERROR
invalid:
	log.Printf("invalid number of arguments in \"%s\" directive",
		name)

	return ERROR
}
