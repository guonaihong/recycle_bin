#uniq命令使用文档
uniq命令消除重复的行.主要与sort命令结合使用

 1. 找出文件中重复的行
    ```shell
    echo -e "1\n2\n3\n4\n3\n"|sort|uniq -d
    #输出
    3
    ```
    
 1. 仅显示唯一的行
    ```shell
    echo -e "1\n2\n3\n4\n3\n"|sort|uniq -u
    #输出
    1
    2
    4
    ```
    
 1. 重复的行只打印一次
    ```shell
    echo -e "1\n2\n3\n4\n3\n"|sort|uniq
    #输出
    1
    2
    3
    4
    ```
    
 1. 找出每行中出现的次数
    ```shell
    echo -e "1\n2\n3\n4\n3\n"|sort|uniq -c
    #输出
    1 
    1 1
    1 2
    2 3
    1 4
    ```

