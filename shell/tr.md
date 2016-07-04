#tr命令使用文档

``` shell
tr [options] s1 s2
```

将来自stdin输入字符s1映射到s2,然后输出stdout,如果s2的长度大于s1超出s1的长度会忽略不计  

 1. 将输入字符串从大写转成小写
 ```shell
    echo "HELLO SHELL" |tr 'A-Z' 'a-z'
    #输出
    hello shell
 ```

 1. 用tr删除字符
 ```shell
    echo "golang 2016-01-02 15:04:05" |tr -d '0-9'
    #输出
    golang -- ::
 ```
 
 1. 用tr删除选定字符集外的字符
 ```shell
    echo "golang 2016-01-02 15:04:05" |tr -d '0-9'
    #输出
    20160102150405
 ```
 
 1. 用tr压缩字符
  * 压缩空格
 ```shell
    echo "wo     shi       shui ge"|tr -s ' '
    #输出
    wo shi shui ge
 ```
 
  * 压缩空行
 ```shell
    echo -e "1\n\n2\n\n3\n\n" |tr -s '\n'
    #输出
    1
    2
    3
 ```
 
 1. example
 求1到100的累加
 ```shell
    seq 100|echo $[ `tr '\n' '+'` + 0 ]
 ```
