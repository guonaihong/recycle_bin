#阅读nginx -s选项代码

首先-h看下帮助信息
sudo /usr/local/nginx/sbin/nginx -h

从帮助信息中可以得知-s选项提供了stop, quit, reopen, reload几个命令
-s signal     : send signal to a master process: stop, quit, reopen, reload

阅读过程:
查找解析命令行的函数ngx_get_options中处理's'选项的代码
```c
// 假如我们用-s stop跑下面的代码(不相关的命令行处理代码已经删除,只保留了-s选项)
static ngx_int_t ngx_get_options(int argc, char *const *argv) {
    u_char     *p; 
    ngx_int_t   i;  

    for (i = 1; i < argc; i++) {
        p = (u_char *) argv[i];

        if (*p++ != '-') { //假如命令行参数为 -s stop,此时判断结束时p就指向为s
            ngx_log_stderr(0, "invalid option: \"%s\"", argv[i]);
            return NGX_ERROR;
        }   

        while (*p) {
            switch (*p++) { //switch判断的值为s,判断结束实际的值指向 '\0'

            case 's':
                if (*p) {   //-s stop的情况不会进, -sstop的情况会进,把值赋给ngx_signal变量
                    ngx_signal = (char *) p;

                } else if (argv[++i]) { //把值赋给ngx_signal变量
                    ngx_signal = argv[i];

                } else {                //假如-s 后面没有跟值,打印帮助信息
                    ngx_log_stderr(0, "option \"-s\" requires parameter");
                    return NGX_ERROR;
                }   

                if (ngx_strcmp(ngx_signal, "stop") == 0
                    || ngx_strcmp(ngx_signal, "quit") == 0
                    || ngx_strcmp(ngx_signal, "reopen") == 0
                    || ngx_strcmp(ngx_signal, "reload") == 0)
                {   
                    ngx_process = NGX_PROCESS_SIGNALLER;
                    goto next; //如果只有-s stop, goto next到达for时就意味着结束
                }   

                ngx_log_stderr(0, "invalid option: \"-s %s\"", ngx_signal);
                return NGX_ERROR;

            default:
                ngx_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
                return NGX_ERROR;
            }   
        }   

    next:

        continue;
    }   

    return NGX_OK;
}
// 跑完这个函数,只设置了两个变量ngx_signal和ngx_process
```
