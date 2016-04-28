#测试一致性哈希#

* 一致性哈希的作用  
 平衡性，把对象均匀地分布在所有桶中。  
 单调性，当桶的数量变化时，只需要把一些对象从旧桶移动到新桶，不需要做其它移动。
* 参与测试的哈希算法 有[31hash](./31hash.c) [33hash](./33hash.c) [一致性哈希](./jump_consistent_hash.c)
* 被测试的文件来自centos系统下/usr/share/dict/words路径下
* 希望得到的结果  
  当桶的数量改变时，只希望一个桶的数据量被影响(数据不一定要求都在一个桶里)
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
 2. 运行脚本
* 20单词，桶从2变为3时的分布 
 
 | 单词 |b=2(31)|单词| b=3(31)|单词| b=2(33)|单词|b=3(33)|单词|b=2(一致性)|单词|b=3(一致性)|
 |---|----|----|----|----|----|---|----|----|----|----|----|
|1080|1	|1080|0	|1080|1	|1080|0	|1080|0	|1080|2|
|10-point|0	|10-point|0	|10-point|0	|10-point|2	|10-point|0	|10-point|2|
|10th|1	|10th|2	|10th|1	|10th|2	|10th|1	|10th|1|
|11-point|1	|11-point|1	|11-point|1	|11-point|2	|11-point|1	|11-point|2|
|12-point|0	|12-point|2	|12-point|0	|12-point|2	|12-point|1	|12-point|1|
|16-point|0	|16-point|2	|16-point|0	|16-point|1	|16-point|0	|16-point|0|
|18-point|0	|18-point|1	|18-point|0	|18-point|0	|18-point|0	|18-point|2|
|1st|0	|1st|1	|1st|0	|1st|2	|1st|1	|1st|2|
|2|0	|2|2	|2|0	|2|2	|2|1	|2|2|
|20-point|1	|20-point|1	|20-point|1	|20-point|1	|20-point|0	|20-point|0|
|2,4,5-t|0	|2,4,5-t|1	|2,4,5-t|0	|2,4,5-t|2	|2,4,5-t|1	|2,4,5-t|1|
|2,4-d|1	|2,4-d|0	|2,4-d|1	|2,4-d|1	|2,4-d|1	|2,4-d|1|
|2D|0	|2D|1	|2D|0	|2D|2	|2D|0	|2D|0|
|2nd|0	|2nd|2	|2nd|0	|2nd|1	|2nd|0	|2nd|0|
|30-30|1	|30-30|0	|30-30|1	|30-30|0	|30-30|1	|30-30|1|
|3-D|0	|3-D|2	|3-D|0	|3-D|2	|3-D|1	|3-D|1|
|3-d|0	|3-d|1	|3-d|0	|3-d|1	|3-d|1	|3-d|1|
|3D|1	|3D|2	|3D|1	|3D|2	|3D|1	|3D|1|
|3M|0	|3M|2	|3M|0	|3M|2	|3M|0	|3M|0|
|3rd|1	|3rd|1	|3rd|1	|3rd|1	|3rd|1	|3rd|1|
仔细对比一致性哈希，可以发现桶(b)从2变为3时，只有6个元素需要改变，达到了只改变一个桶的需要 m(元素总个数20) / b(3) = 6，
常见的字符串31和33hash算法，有16个元素需要改变。




* 结果
 TODO hello

##清理##
``` shell
make clean
```
