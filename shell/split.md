# split命令使用文档

 1. 按指定的大小分割文件
    ```shell
    #-b后面的数字可以带单位,k,m. k就是一次分割n KB. m就是一次分割n MB
    split -b 10 test
    ls
    test    xaa xab xac xad xae xaf xag xah xai xaj
    ```

 1. 按文件行数分
    ```shell
    split -l 1 test 
    guodeMacBook-Air:test guo$ ls
    test    xaa xab xac xad xae
    ```

 1. 跳过某个文件的n字节
 ```shell
 split -b 20 test
 rm xaa
 cat x* >new.file
 ```
