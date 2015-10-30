#!/bin/bash

if [ ! -f parse_page ];then
    make
fi
for html in ../../dump/*;do
    #echo $html
    parse_page $html
done

