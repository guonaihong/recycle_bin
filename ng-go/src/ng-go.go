package main

import (
	"core"
	"flag"
	"fmt"
	_ "stream"
)

func main() {
	confName := flag.String("c", "", "set configuration file")
	flag.Parse()

	if *confName == "" {
		flag.Usage()
		return
	}

	c := core.Cycle{}
	core.CycleInit(&c, *confName)

	fmt.Printf("parse ok\n")
}
