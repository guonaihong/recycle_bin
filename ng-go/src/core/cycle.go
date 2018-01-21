package core

import (
	"log"
	"sync"
)

type Cycle struct {
	Plugins
	Listeing []Listening
}

func CycleInit(c *Cycle, confName string) error {

	var wg sync.WaitGroup
	log.Printf("cycle init\n")
	c.Plugins = plugins

	cf := Conf{CmdType: MAIN_CONF,
		Cycle: *c}

	ConfParse(&cf, confName)

	log.Printf("current listen data is %s\n", c.Listeing)
	for _, v := range c.Listeing {
		wg.Add(1)
		go func() {
			defer wg.Done()
			v.TcpLoop()
		}()
	}

	wg.Wait()
	return nil
}
