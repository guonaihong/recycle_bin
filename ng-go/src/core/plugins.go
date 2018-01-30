package core

import (
	"log"
	"sync"
)

type Plugins struct {
	sync.RWMutex

	MainModulesToPos map[string]int32

	Modules [][]*Module

	CommandsToPos []map[string][]int

	Commands [][]Command
}

type cToPos map[string][]int

const MAIN_MODULE = 5

var plugins = Plugins{
	MainModulesToPos: make(map[string]int32, 20),
	Modules:          make([][]*Module, 0, MAIN_MODULE),
	CommandsToPos:    make([]map[string][]int, 0, MAIN_MODULE),
	Commands:         make([][]Command, 0, MAIN_MODULE),
}

func Register(m *Module) {
	plugins.Lock()
	defer plugins.Unlock()

	if m == nil {
		panic("core: Register modules is nil")
	}

	var commandsToPos map[string][]int
	var modules []*Module
	var commands []Command

	mainPos, ok := plugins.MainModulesToPos[m.MainName]

	if !ok {
		commandsToPos = make(map[string][]int, 1)

		mainPos = int32(len(plugins.Modules))

		plugins.MainModulesToPos[m.MainName] = mainPos

		plugins.Modules = append(plugins.Modules, make([]*Module, 3))
		plugins.CommandsToPos = append(plugins.CommandsToPos, make(map[string][]int, 3))
		plugins.Commands = append(plugins.Commands, make([]Command, 0, 3))
	} else {
		modules = plugins.Modules[mainPos]
		commands = plugins.Commands[mainPos]
		commandsToPos = plugins.CommandsToPos[mainPos]
	}

	for _, v := range modules {
		if v.Name == m.Name && v.Type == m.Type {
			panic("core: Register called twice for modules " + m.Name)
		}
	}

	modules = append(modules, m)
	modules[len(modules)-1].CtxIndex = len(modules) - 1

	for _, v1 := range commands {
		for _, v2 := range m.Commands {
			if v1.Name == v2.Name && v1.Type == v2.Type {
				panic("core: Register called twice for commands " + v1.Name)
			}
		}
	}

	for _, v := range m.Commands {
		v.CtxIndex = uint(len(modules) - 1)
		commands = append(commands, v)
		commandsToPos[v.Name] = append(commandsToPos[v.Name], len(commands)-1)

	}

	log.Printf("main module name = %s, moudle name = %s, commands = %d\n",
		m.MainName, m.Name, len(commands))

	plugins.Modules[mainPos] = modules
	plugins.CommandsToPos[mainPos] = commandsToPos
	plugins.Commands[mainPos] = commands

	//log.Printf("%v\n", plugins.Commands)
	//log.Printf("%v\n%v", m, plugins)
}

func (p *Plugins) GetMainModules(mainModule string) []*Module {
	pos, ok := p.MainModulesToPos[mainModule]
	if !ok {
		return nil
	}
	return p.Modules[pos]
}

func (p *Plugins) GetMainModules2(pos int) []*Module {
	p.RLock()
	defer p.RUnlock()
	return p.Modules[pos]
}

func (p *Plugins) GetCommand(name string, t uint) (*Command, int) {
	p.RLock()
	defer p.RUnlock()

	for k, v := range p.CommandsToPos {
		pos, ok := v[name]
		if !ok {
			continue
		}

		for _, i := range pos {
			cmd := p.Commands[k][i]

			/*
				if name == "proxy_pass" {
					log.Printf("plugin....###############3 %#v:%v:type(%x)\n", cmd, (cmd.Type&t) == 0, t)
				}
			*/

			if (cmd.Type & t) == 0 {
				continue
			}
			return &cmd, i
		}
	}

	return nil, 0
}
