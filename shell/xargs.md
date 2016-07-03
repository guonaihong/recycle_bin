# xargs命令使用文档
xargs命令把从stdin接收到的数据重新格式化,再将其作为参数提供给其他命令  

###### 将多行输入转成单行输出
```shell
echo -e "1 2 3 4 5\n6 7 8 9\n10 11 12"|xargs
#输出
1 2 3 4 5 6 7 8 9 10
```
###### 将单行输出转成多行输出
```shell
echo "1 2 3 4 5 6 7 8 9 10"|xargs -n 3
#输出
1 2 3
4 5 6
7 8 9
10
```

###### 使用自定义的分隔符.用-d选项可以指定
```shell
echo -n 'mail:x:8:12:mail:/var/spool/mail:/sbin/nologin'|xargs -d:
#输出
mail x 8 12 mail /var/spool/mail /sbin/nologin

#每行输出一个
echo -n 'mail:x:8:12:mail:/var/spool/mail:/sbin/nologin'|xargs -d: -n1
#输出
mail
x
8
12
mail
/var/spool/mail
/sbin/nologin

```

######格式化参数
可以编写如下代码,作用很简单打印命令行参数,并以$接尾.  
```c
#include <stdio.h>
int main(int argc, char **argv) {
    while (*++argv) {
        printf("%s ", *argv);
    }
    if (argc > 1)
        printf("$\n");
    return 0;
}

#保存成t.c
#gcc t.c 生成a.out

./a.out 1 2 3 4
#输出,把命令行的参数都打印了出来
1 2 3 3
```

 * 有3个问题需要解决
```shell
#假如有一个命令生成参数列表,需要将这些参数传递给一个命令(这里是a.out),每次提供一个参数
./a.out arg1
./a.out arg2
./a.out arg3
./a.out arg4
./a.out arg5
./a.out arg6
#或者每次传递两个参数
./a.out arg1 arg2
./a.out arg3 arg4
./a.out arg5 arg6
#或者一下次全部传递过去
./a.out arg1 arg2 arg3 arg4 arg5 arg6
```
 * 解决方式
先看生成参数列表的命令
```shell
seq 6|sed 's/^/arg/'
#输出
arg1
arg2
arg3
arg4
arg5
arg6
arg7
arg8
arg9
arg10

#上面第一个问题,我们可以将这个命令多次执行,每次执行使用一个参数
seq 6|sed 's/^/arg/'|xargs -n 1 ./a.out
#输出
arg1 $
arg2 $
arg3 $
arg4 $
arg5 $
arg6 $

#上面第二个问题,多次执行命令,每次使用两个
seq 6|sed 's/^/arg/'|xargs -n 2 ./a.out
#输出
arg1 arg2 $
arg3 arg4 $
arg5 arg6 $

#每次执行需要x个参数的命令时,使用
INPUT | xargs -n X

#为某个执行命令一次性提供所有的参数
seq 6|sed 's/^/arg/'|xargs ./a.out

#输出
arg1 arg2 arg3 arg4 arg5 arg6 $

#如果送给./a.out的参数有固定的部分也有变化的部分
./a.out -n arg1 -s
./a.out -n arg2 -s
./a.out -n arg3 -s

#这时候可以使用xargs 的-I选项指定替换字符串,这个字符串在xargs扩展时自动替换掉
#注意-I后面的字符串是可以自定义的

seq 6|sed 's/^/arg/'|xargs -I xx  ./a.out  ./a.out -n xx -s 
#输出
./a.out -n arg1 -s $
./a.out -n arg2 -s $
./a.out -n arg3 -s $
./a.out -n arg4 -s $
./a.out -n arg5 -s $
./a.out -n arg6 -s $
```
######example
```shell
#简单地计算下当前录下一共多少c语言代码
find . -type f -name '*.c' -exec cat {} \;|wc -l

#删除vim的临时文件
find . -type f -name '*swp' -print0|xargs -0 rm -f

#等同上面一种写法
find . -type f -name '*swp' -exec rm {} \;
```
