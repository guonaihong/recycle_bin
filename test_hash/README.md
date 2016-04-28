#测试一致性哈希#

* 一致性哈希的作用  
 平衡性，把对象均匀地分布在所有桶中。  
 单调性，当桶的数量变化时，只需要把一些对象从旧桶移动到新桶，不需要做其它移动。
* 参与测试的哈希算法 有[31hash](./31hash.c) [33hash](./33hash.c) [一致性哈希](./jump_consistent_hash.c)
* 被测试的文件来自centos系统下/usr/share/dict/words
* 希望得到的结果  
  当桶的数量改变时，只希望一个桶的数据被影响(不一定要求在一个桶里)
* 测试的脚本含义
```shell
#取20个单词
time head -20 /usr/share/dict/words|
      #删除单词中的单引号
      sed "s/'//g"|
        #./31hash 是31hash的可执行文件，被测试的hash算法已经编译成一个可执行文件，
        #第一个参数是单词，第二个参数是桶的大小
        xargs -I {} ./31hash {} 2 &>31_data/31_buckets_2
        
#time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./31hash {} 3 &>31_data/31_buckets_3
#time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./33hash {} 2 &>33_data/33_buckets_2
#time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./33hash {} 3 &>33_data/33_buckets_3
```
* 开始测试
 1. make 
 2. TODO
* 结果
 TODO hello

##清理##
``` shell
make clean
```
