#阅读nginx -s选项代码

首先-h看下帮助信息
sudo /usr/local/nginx/sbin/nginx -h

从帮助信息中可以得知-s选项提供了stop, quit, reopen, reload几个命令  
-s signal     : send signal to a master process: stop, quit, reopen, reload

问题:  
1. nginx stop, quit, reopen, reload命令实际使用的posix信号名是?
1. nginx -s 选项如何知道该往哪个进程发信号, 是如何获取pid的?
1. nginx 如何接收这些信号以及对应的处理?

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

只要找到ngx_signal变量就知道实际干活的函数
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
```c
ngx_get_options
ngx_signal_process
ngx_os_signal_process
```

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

nginx -s 选项如何知道该往哪个进程发信号, 是如何获取pid的?

这个答案还要在ngx_signal_process函数里找, ngx_signal_process代码静下心里看,  
总结为打开某个神秘的文件,然后获得了pid。然后用kill函数发送信号,下面是我去掉
打印语句的ngx_signal_process函数
``` c
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig) {
    ssize_t           n;
    ngx_int_t         pid;
    ngx_file_t        file;
    ngx_core_conf_t  *ccf;
    u_char            buf[NGX_INT64_LEN + 2];

    //这个地方获取了神秘文件名,在本函数里要想知道这句话的意思比较难
    //接下来看下cycle是怎么来的
    ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);

    ngx_memzero(&file, sizeof(ngx_file_t));

    file.name = ccf->pid;
    file.log = cycle->log;
                                                                                                   
    //打开存放pid的文件
    file.fd = ngx_open_file(file.name.data, NGX_FILE_RDONLY,
                            NGX_FILE_OPEN, NGX_FILE_DEFAULT_ACCESS);
                                                                                                   
    if (file.fd == NGX_INVALID_FILE) {
        return 1;
    }
                                                                                                   
    n = ngx_read_file(&file, buf, NGX_INT64_LEN + 2, 0);//读取文件内容

    if (ngx_close_file(file.fd) == NGX_FILE_ERROR) {//关闭文件
    }

    if (n == NGX_ERROR) {
        return 1;
    }
                                                                                                   
    while (n-- && (buf[n] == CR || buf[n] == LF)) { /* void */ }//跳过尾部的换行符
                                                                                                   
    pid = ngx_atoi(buf, ++n);//把字符串转成数字
                                                                                                   
    if (pid == NGX_ERROR) {
        return 1;
    }

    return ngx_os_signal_process(cycle, sig, pid);
}
```
只要知道ngx_signal_process第一个参数cycle是怎么样来的，就有可能知道神秘文件是怎么来的  
ngx_get_conf(cycle->conf_ctx, ngx_core_module)展开就是cycle->conf_ctx[ngx_core_module.index]  

通过搜索cycle知道该变量是由ngx_init_cycle函数初始化的,通过在ngx_init_cycle函数里面搜索conf_ctx  

发现cycle->conf_ctx[ngx_core_module.index] 是如何赋值的,请看如下代码,最终发现  
ngx_modules->ctx成员藏有钩子函数
``` c
 for (i = 0; ngx_modules[i]; i++) {
        if (ngx_modules[i]->type != NGX_CORE_MODULE) {
            continue;
        }

        module = ngx_modules[i]->ctx;//取出ctx成员

        if (module->create_conf) {
            rv = module->create_conf(cycle);//调用create_conf钩子函数
            if (rv == NULL) {
                ngx_destroy_pool(pool);
                return NULL;
            }
            cycle->conf_ctx[ngx_modules[i]->index] = rv; //赋值
        }
    }

```
重新回到ngx_signal_process函数里ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);  
打开ngx_core_module模块里的ctx成员搜索pid，最后定位到如下代码,最终迷题解开,打开的文件名是logs/nginx.pid  

``` c
#define NGX_PID_PATH  "logs/nginx.pid"
static char *ngx_core_module_init_conf(ngx_cycle_t *cycle, void *conf) {

    if (ccf->pid.len == 0) {
        ngx_str_set(&ccf->pid, NGX_PID_PATH);
    }
}

```
