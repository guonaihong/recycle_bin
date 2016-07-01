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
#修改json的key名
echo '{"user":"stedolan","titles":["JQ Primer", "More JQ"]}'|jq '{user1: .user, title2: .titles}'
#输出
{
  "user1": "stedolan",
  "title2": [
    "JQ Primer",
    "More JQ"
  ]
}

#如果其中一个表达式产生多个结果,那最终生成的json也有多个结果
#其中.titles[]会查询出两个结果,那最终生成的json也是两个
echo '{"user":"stedolan","titles":["JQ Primer", "More JQ"]}'|jq '{user, titles:.titles[]}'
#输出
{
  "user": "stedolan",
  "titles": "JQ Primer"
}
{
  "user": "stedolan",
  "titles": "More JQ"
}

#如果想使用原来的json某个key的值作新的json的key,可以使用(.key)语法
echo '{"user":"stedolan","titles":["JQ Primer", "More JQ"]}'| jq '{(.user): .titles}'
#输出
{
  "stedolan": [
    "JQ Primer",
    "More JQ"
  ]
}
```

#####运算符
jq的运算符会根据参数(数字,数组,字符串)的不同,有不同的形为,jq不会做隐式类型转换,如果把string添加到一个object   
就会报错  

######加法+运算符
+运算符需要两个相同输入,并把结果加在一起  
 * 数字常规的加法
 * array拼接成一个大的数组
 * string拼接成一个大的string
 * object也是合并操作,如果有两个key相同的object新的覆盖旧的

null可以与任何值相加,返回另外一个值  
#数字相加
```shell
echo '{"a":1}'|jq '.a + 1'
#输出
2

#array相加
echo '{"a": [1,2], "b": [3,4]}'|jq '.a+.b'
#输出
[
  1,
  2,
  3,
  4
]

#string相加
echo '{"a": "hello", "b": "world"}'|jq '.a+.b'
#输出
"helloworld"

#object相加
echo null|jq '{a: 42} + {b: 2} + {c: 3} + {a: 1}'
#输出
{
  "a": 1,
  "b": 2,
  "c": 3
}

#有空值相加的情况
echo '{"a": 1}'|jq 'null +.a'
#输出
1

echo '{}'|jq '.a+1'
#输出
1
```

######减法-运算符
-号运算符用于数字,用于数组,会在第一个数组删除第二个数组中出现的所有项  
```shell
#数字相减
echo '{"a":4}'|jq '4 - .a'
#输出
0

#数组相减
echo '  ["xml", "yaml", "json"]' |jq '. - ["xml", "json"]'
#输出
[
  "yaml"
]
```

######乘法*除法运算符
* /只能用在数字类型上  
```shell
echo 5|jq '10 / . * 3'
#输出
6
```

######length
length用于不同类型值的长度  
 * string:返回字符串中字符的个数,如果有中文返回中文的个数
 * array: 返回数组元素的个数
 * object: 返回键-值对的个数

```shell
echo '["郭", [1,2], "string", {"a":2}, null]'| jq '.[]|length'
#输出
1
2
6
1
0
```

######keys and keys_unsorted
keys可以返回json键名数组,其中keys与keys_unsorted区别是keys返回的数组是排过序的  
keys_unsorted返回的数组是不排序  
当json的顶层元素是数组时,keys返回数组的下标

```shell
echo '{"abc": 1, "abcd": 2, "Foo": 3}'|jq 'keys'
#输出
[
  "Foo",
  "abc",
  "abcd"
]

echo '{"abc": 1, "abcd": 2, "Foo": 3}'|jq 'keys_unsorted'
#输出
[
  "abc",
  "abcd",
  "Foo"
]

echo '["aaa", "bbb", "ccc"]'|jq 'keys'
#输出
[
  0,
  1,
  2
]

```
