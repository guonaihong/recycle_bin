#sort命令使用文档
sort主要是对文件或者stdin进行排序  

 1. 排序
    ```shell
    sort file2 > sorted.txt
    ```

 1. 对某个文件排序并修改原文件
    ```shell
    sort file2 -o sorted.txt
    ```

 1. 排照数字排序
    ```shell
    #sort 默认以字典排序,有时候不是期望的输出
    seq 10 |sort
    1
    10
    2
    3
    4
    5
    6
    7
    8
    9
    #如果希望按数字排序,可以使用-n选项
    seq 10|sort -n
    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
    ```

 1. 按照逆序排序
    ```shell
    seq 10|sort -rn
    10
    9
    8
    7
    6
    5
    4
    3
    2
    1
    ```

 1. 合并两个已排序过的文件
    ```shell
    sort -m sorted.txt sorted2.txt
    ```

 1. 找出已排序文件中不重复的行
    ```shell
    sort sorted.txt|uniq
    ```

 1. sort以'\0'作为分隔符与xargs -0相兼容
    ```shell
    sort -z data.txt|xargs -0
    ```

 1. sort 选项-b用于忽略文件中的前空白行
    ```shell
    sort -b data.txt
    ```

#example
 * sort多字段排序
    ```shell
    cat version
    1.2.1
    1.2.10
    1.2.2
    2.1.1
    2.10.2
    2.2.1
    3.0.01
    30.0.0
    4.0.0

    cat version|sort version -n -t'.' -k1,1 -k2,2 -k3,3
    ```
