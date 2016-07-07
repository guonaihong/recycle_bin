#md5sum命令使用手册

 1. 使用md5
     * 计算md5
     
     ```shell
        md5sum file &>file_name.md5
     ```
     * 检查md5
     
     ```shell
        md5sum -c file_name.md5
     ```
 1. 使用gpg
     * 对文件进行加密
     
     ```shell
     gpg -c filename
     ```
     * 对文件进行解密
     
     ```shell
     gpg filename.gpg
     ```
 1. 使用base64
     * 编码
     
     ```shell
     base64 filename >outfile
     #or
     cat file|base64 >outfile
     ```
     * 解码
     
     ```shell
     base64 -d file > outfile
     cat file|bashe -d outfile
     ```
