#du命令使用文档
du主要统计某个文件或者目录所占磁盘空间大小

 1. -a 递归统计某个目录
    ```shell
    du -a dirname
    ```

 1. -h 显示时以KB或者MB或者GB为单位显示出磁盘使用的情况
    ```shell
    du -h du.md
    4.0K    du.md

    ```

 1. -s 只输出合计数据,计算当前目录磁盘占用
    ```shell
    du -sh .
    3.6M    .
    ```

 #exampele
 * 找出当前目录里最大最大的10个文件
    ```shell
    find . -type f -exec du {} \;|sort -nrk1|head
    688 ./mini_ant/.git/objects/pack/pack-0ac94dfdcb944a62a7b5e5dd1bbd4b3348eb6c4e.pack
    144 ./.git/objects/pack/pack-1aa7de3619de7b9c88323170359198f94723c1df.pack
    48  ./.git/index
    40  ./mini_ant/src/mini_ant.c
    32  ./mini_ant/src/ant_dns.c
    32  ./mini_ant/.git/objects/pack/pack-0ac94dfdcb944a62a7b5e5dd1bbd4b3348eb6c4e.idx
    24  ./use_libnids/parse_smtp_clent.c
    24  ./shell/jq.md
    24  ./shell/.sort.md.swp
    24  ./shell/.du.md.swp
    ```shell
