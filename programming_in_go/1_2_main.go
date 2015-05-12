package main

import (
    "./stack"
    "fmt"
)

func main() {
    var s stack.Stack
    s.Push(1)
    s.Push(1.1)
    s.Push("3333")
    s.Push([]byte("44444"))
    s.Push("555555555")

    for {
        v, e := s.Pop()
        if e != nil {
            break
        }
        fmt.Println(v)
    }
}
