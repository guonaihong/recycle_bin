#测试一致性哈希#

##准备工作##
make

time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./31hash {} 2 &>31_data/31_buckets_2
time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./31hash {} 3 &>31_data/31_buckets_3

time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./33hash {} 2 &>33_data/33_buckets_2
time head -20 /usr/share/dict/words|sed "s/'//g"|xargs -I {} ./33hash {} 3 &>33_data/33_buckets_3

##清理##
make clean
