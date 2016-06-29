# jq命令使用文档

###### 安装jq
```shell
# 打开https://github.com/stedolan/jq/releases
# 找到Source code (tar.gz),点击并下载
tar xvf 压缩包
cd jq目录
./configure && make && sudo
```

###### 格式化json
不加任何选项和表达式的作用是格式化json字符串,比较实用  
```shell
echo '{"key":"va", "key2":"val2"}'|jq  #格式化echo里面的json
cat t.txt|jq                           #格式化t.txt里面的json
jq . t.txt                             #jq打开文件,并格式化t.txt里面的json
```

###### 根据key查询json的值
根据key查询json可以写成.key,如果key是特殊的字符,比如全数字需要用引号括起来: ."key"  
.foo.bar的形式类似于shell的管道符|,.foo.bar等于.foo|.bar  
````shell
echo '{"foo": 42, "bar": "less interesting data"}'|jq '.foo'
42    #如果key对应的val存在则输出val

echo '{"notfoo": true, "alsonotfoo": false}'      |jq '.foo'
null  #如果key对应的val值不存在则输出null

echo '{"111":"222", "333":"444"}'|jq '.["111"]'
"222" #输出

echo '{"111":"222", "333":"444"}'|jq '."111"' #效果和上面一条一样
"222" #输出
```
##### 查询json里面的数组
.[index]查询一个值,需要注意的是,json的数组是第0开始计数的, .[index]查询第index+1个元素  
.[2]就是查询第3个元素  

.[start:end]选取一个范围内的数组,包含start不包含end  
.[2:10]就是查询下标为2(包含下标2)  

.[:end]选取end之前的元素(不包含end索引)
.[:2]输出就是下标为0,1的元素

.[-index:] index前面有-号,表示倒过来的第index个,这时候的-index:是从-1开始计数的后面的元素
.[-2:]就是倒数第1到2元素的所有元素

.[-index] 倒数第index个元素
到下标到10(不包含10)范围内的数组  
```shell
#查询数组第1个元素
echo '[{"name":"JSON", "good":true}, {"name":"XML", "good":false}]' |jq '.[0]'
#输出
{
  "name": "JSON",
  "good": true
}

#查询第3个元素,需要注意的是数组的下标是从0开始算的
'[{"name":"JSON", "good":true}, {"name":"XML", "good":false}]' |jq '.[2]'
#输出
null

#查询下标为2到3之间的元素
echo '["a","b","c","d","e"]'|jq '.[2:4]'
#输出
[
  "c",
  "d"
]

#查询下标为3之前的元素
echo '["a","b","c","d","e"]'|jq '.[:3]'
#输出
[
  "a",
  "b",
  "c"
]

#查询倒数第1-2个元素
echo '["a","b","c","d","e"]'|jq '.[-2:]'

#输出
[
  "d",
  "e"
]

#查询倒数第2个元素
jq '.[-2]'

#输出
"d"
```

######查询对象的所有值.[]
可以使用.[]语法,查询json对象的所有值

```shell
#拿到key的val以及key2的val2
echo '{"key":"val", "key2":"val2"}'|jq '.[]'
#输出
"val"
"val2"

#拿到key的val(一个object),key2的val
echo '{"key":{"key3":"val3", "key4":"val4"}, "key2":"val2"}'|jq '.[]'
#输出
{
  "key3": "val3",
  "key4": "val4"
}
"val2"

#拿到json数组里面的值
echo ' [{"name":"JSON", "good":true}, {"name":"XML", "good":false}]'|jq '.[]'
#输出
{
  "name": "JSON",
  "good": true
}
{
  "name": "XML",
  "good": false
}
```

######多条件使用,分开
如果要写多个过滤条件使用,号,现输出,左边的结果在输出,右边的结果  
```shell
echo '{"foo": 42, "bar": "something else", "baz": true}'|jq '.foo,.bar,.baz'
#输出
42
"something else"
true

#.key和[]表达式可以组合使用
echo '{"user":"stedolan", "projects": ["jq", "wikiflow"]}'|jq '.user,.projects[]'
#输出
"stedolan"
"jq"
"wikiflow"

#可以使用,一次查询数组里的多个元素
echo '["a","b","c","d","e"]'|jq '.[4,3]'
#输出
"e"
"d"
```

######管道符号|
shell里面的|是连接各个shell命令的通道,像大管道套小管道,过滤器就是命令,可以很方便的过滤出想要的数据来  
jq里面也有|符号  
```shell
#可以先用.[]拿到值,再使用|(管道)拿到name
echo '[{"name":"JSON", "good":true}, {"name":"XML", "good":false}]'|jq '.[] | .name'
#输出
"JSON"
"XML"

######把查询结果包装成一个数组(array)--使用[]符号
[]在jq里面表示数组,可以现查询再使用[]把查询结果包装成数组
echo '[{"name":"JSON", "good":true}, {"name":"XML", "good":false}]'|jq '[.[]|.name]'
#输出
[
  "JSON",
  "XML"
]

```

######把查询结果包装成一个对象(object)--使用{}符号

```shell
```
