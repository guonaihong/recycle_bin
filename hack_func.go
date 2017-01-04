package main

import (
    "fmt"
    "time"
)

//1 start
type Transport struct {
    Dial func(addr string) error
}

func (t Transport) do(addr string) {
    t.Dial(addr)
}

//1 end

//2 start
type Dialer struct {
    Timeout time.Duration
}

func (d *Dialer) Dial(addr string) error {
    fmt.Printf("to call dial...timeout(%v):addr(%s)\n", d.Timeout, addr)
    return nil
}

//2 end

func main() {
    //fmt.Printf("%p\n", (&test{}).test1)
    //t := Transport{y: (&Dialer{Timeout: 5 * time.Second}).y}
    t := Transport{Dial: (&Dialer{Timeout: 5 * time.Second}).Dial}
    //t := Transport{}
    t.do("127.0.0.1")
}
