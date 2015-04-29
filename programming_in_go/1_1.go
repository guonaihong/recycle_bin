package main

import (
    "fmt"
    "os"
)

func main() {
    if len(os.Args) != 2 {
        fmt.Printf("%s number\n", os.Args[0])
        return
    }

    bigdigits := [][]string{
        {"  000 ",
        " 0     0",
        " 0     0",
        " 0     0",
        "   000  ",
        },
        {
        "   1   ",
        " 1 1   ",
        "   1   ",
        "   1   ",
        "1111111",
        },
        {
        "222222 ",
        "     2 ",
        "222222 ",
        "2      ",
        "222222 ",
        },
        {
        "333333 ",
        "     3 ",
        "333333 ",
        "     3 ",
        "333333 ",
        },
        {
        "4    4",
        "4    4",
        "444444",
        "     4",
        "     4",
        },
        {
        "555555",
        "5     ",
        "555555",
        "     5",
        "555555",
        },
        {
        "666666",
        "6     ",
        "666666",
        "6    6",
        "666666",
        },
        {
        "777777",
        "     7",
        "     7",
        "     7",
        "     7",
        },
        {
        "888888",
        "8    8",
        "888888",
        "8    8",
        "888888",
        },
        {
        "999999",
        "9    9",
        "999999",
        "     9",
        "999999",
        },
    }

    for row := range bigdigits[0] {
        strdigit := os.Args[1]
        line := ""
        for i := range strdigit {
            d := strdigit[i] - '0'
            if d < 0 || d > 9 {
                fmt.Printf("error number\n")
                return 
            }
            line += bigdigits[d][row] + " "
        }
        fmt.Printf("%s\n", line)
    }
}
