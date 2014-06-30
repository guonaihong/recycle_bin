#!/bin/bash
#命令的输出作为shell的输入,很强大的技巧,第3行
find mrcp/ -maxdepth 2 -type d |
    perl -lne 'print if s/(.*?\/)(\w+$)/mkdir $1ho_$2/'|
        bash
