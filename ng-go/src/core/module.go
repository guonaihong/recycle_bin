package core

/*
import (
	"core/conf"
)
*/

import (
	//"core/conf"
	"sync"
)

var (
	modulesMu sync.RWMutex
	modules   = make(map[string][]Command)
)

type Module struct {
	Ctx      interface{}
	CtxIndex int
	Name     string //module name
	Commands []Command
	Type     uint
	MainName string
}
