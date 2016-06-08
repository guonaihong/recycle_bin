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

只要找到ngx_signal表量就知道实际干活的函数
```c
    //如果ngx_signal有值，则调用ngx_signal_process
    if (ngx_signal) {
        return ngx_signal_process(cycle, ngx_signal);
    }

    //ngx_signal_process实际调用ngx_os_signal_process
```

```c
ngx_int_t ngx_os_signal_process(ngx_cycle_t *cycle, char *name, ngx_int_t pid) {
    ngx_signal_t  *sig;

    for (sig = signals; sig->signo != 0; sig++) {
        if (ngx_strcmp(name, sig->name) == 0) {
            if (kill(pid, sig->signo) != -1) { //实际是通过kill命令发送信号
                return 0;
            }
            //出错记录日志
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "kill(%P, %d) failed", pid, sig->signo);
        }
    }

    return 1;
}

```
对于发送端的流程走的下面3个函数
ngx_get_options
ngx_signal_process
ngx_os_signal_process

现在分析stop, quit, reopen, reload命令实际使用了哪个信号
答案在ngx_signal_t声明的signals表里
``` c
ngx_signal_t  signals[] = {
    { ngx_signal_value(NGX_RECONFIGURE_SIGNAL),
      "SIG" ngx_value(NGX_RECONFIGURE_SIGNAL),
      "reload",
      ngx_signal_handler },

    { ngx_signal_value(NGX_REOPEN_SIGNAL),
      "SIG" ngx_value(NGX_REOPEN_SIGNAL),
      "reopen",
      ngx_signal_handler },

    { ngx_signal_value(NGX_TERMINATE_SIGNAL),
      "SIG" ngx_value(NGX_TERMINATE_SIGNAL),
      "stop",
      ngx_signal_handler },

    { ngx_signal_value(NGX_SHUTDOWN_SIGNAL),
      "SIG" ngx_value(NGX_SHUTDOWN_SIGNAL),
      "quit",
      ngx_signal_handler },

    { 0, NULL, "", NULL }
};

//ngx_signal_value的作用是宏替换是作字符拼接SIG和ngx_signal_value
//如果ngx_signal_value(QUIT)，那么展开的值就是SIGQUIT
#define NGX_RECONFIGURE_SIGNAL   HUP

#if (NGX_LINUXTHREADS)
#define NGX_REOPEN_SIGNAL        INFO
#else
#define NGX_REOPEN_SIGNAL        USR1
#endif

#define NGX_TERMINATE_SIGNAL     TERM
#define NGX_SHUTDOWN_SIGNAL      QUIT

//从上面的宏替换可以看出,左边是nginx命令,右边是信号名
//reload-->SIGHUP
//reopen-->SIGUSR1
//stop  -->SIGTERM
//quit  -->SIGQUIT

```
