
/*
 *Copyright (C) guonaihong
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>

#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <pthread.h>
#include <iconv.h>
#include "ant_queue.h"
#include "ant_array.h"
#include "ant_socket.h"
#include "ant_dns.h"
#include "ant_buf.h"
#include "ant_macro.h"
#include "ant_http.h"
#include "ant_log.h"
#include "ant_hash.h"
#include "ant_url.h"
#include "ant_context.h"

#define DUMP_DIR  "dump"
#define LOG_DIR  "log"

int http_event_loop(total_context_t *total);

static int page_to_utf8(ant_buf_t *dst, ant_buf_t *src, const char *fromcode);

int html_dump(struct http_info *info, total_context_t *total);

struct http_info *http_new(down_t *down);

void http_init(struct http_info *info, down_t *down);

void http_clean(struct http_info *info);

int file_extract_url(char *fname, total_context_t *total);

static int read_config(char *fname);

static void dir_init(void);

int skip_head(char *url) {
    if (
            !strncasecmp(url, "http://", 7)
            || !strncasecmp(url, "https://", 8)
        )
        return url[4] == 's' ? 8 : 7;

    return 0;
}

static void usage(char *argv) {
    fprintf(stderr, "usage: %s\r\n"
            "\t-i, --index url\r\n"
            "\t-f, --file  urlfile\r\n"
            "\t-d, --debug \r\n"
            "\t-h, --help  \r\n"
            , argv);
    exit(1);
}

void print_status(int signo);

static void cleanup(int status, void *arg) {
    ant_log_clean();
    if (status == EXIT_FAILURE)
        print_status(-1);
}

int down_add(unsigned *ip, unsigned short port, char *url, struct queue_head *queue);

void url_add(char *url, total_context_t *total);

void filter_add(char *url2, total_context_t *total);

static struct status_info status = {0, 0, 0, 0, 0};

typedef struct ant_option {
    int debug;
    int type;
    int maxconn;
}opt_t;

opt_t opt = {0};

int main( int argc, char **argv ) {

    struct sigaction act;
    const char *opstring = "i:f:hd";
    const struct option longopts[] = {
        {"index", 1, NULL, 'i'},
        {"file", 1, NULL, 'f'},
        {"help", 0, NULL, 'h'},
        {"debug", 0, NULL, 'd'},
    };

    char      c;
    int   check;

    check = 0;

    status.btime = time(NULL);
    act.sa_handler = print_status;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    total_context_t total;
    pthread_t id;

    /* init */
    dir_init();
    uniq_init();
    ant_log_init();
    ant_total_context_init(&total);

    /* init total->cache_url*/
    cache_url_init(&total, ANT_LOG_RECORD_NAME);

    while ((c = getopt_long(argc, argv, opstring, longopts, NULL)) != -1) {
        switch(c) {
            case 'i':
                filter_add(optarg, &total);
                check = 0x001;
                break;
            case 'f':
                file_extract_url(optarg, &total);
                check = 0x002;
                break;
            case 'h':
                usage(argv[0]);
                break;
            case 'd':
                fprintf(stderr, "open the debug option\n");
                opt.debug = 1;
                break;
            default:
                usage(argv[0]);
                exit(0);
        }
    }

    if (check == 0) {
        usage(argv[0]);
        exit(0);
    }

    on_exit(cleanup, NULL);
    pthread_create(&id, NULL,thread_dns_main, &total);

    read_config("conf/ant.conf");
    if (http_event_loop(&total) == -1) {
        exit(EXIT_FAILURE);
    }

    pthread_join(id, NULL);
    return 0;
}

void print_status(int signo) {

    int val;
    status.etime = time(NULL);
    val = (status.etime - status.btime);
    fprintf(stderr, "\r\n\r\n\r\n"
            "queue_num      should_down    total_down    runtime    speed\r\n"
            "%d             %d             %d            %d         %2lf\n", 
            status.queue_num,status.should_down,status.total_down, val, 
            (double)status.total_down / val);

    exit(0);
}

static void dir_init(void) {
    if (!DIR_EXISTS(DUMP_DIR))
        mkdir(DUMP_DIR, 0755);

    if (!DIR_EXISTS(LOG_DIR))
        mkdir(LOG_DIR, 0755);
}

static int do_create(struct http_info *info, int efd, ant_array_t *socks) {

    struct epoll_event event;
    struct sockaddr_in peer;
    int    connfd;
    unsigned ip;

    ip = info->down->ip2 == 0 ? *(info->down->ip) : info->down->ip2;

    connfd = sock_nonblock_create(&peer, ip,(info->down->port == 0) ? 80 : info->down->port);

    if (connfd == 0) {
        fprintf(stderr, "connfd == %d,url = %s\n", connfd, info->down->url);
        exit(1);
    }

    if (connect(connfd, (struct sockaddr*)&peer, sizeof(peer)) < 0 ) {
        if (errno != EINPROGRESS) {
            ant_debug_sock_url(connfd, info->down->url);
            perror("socket");
            return -1;
        }
    }

    info->fd = connfd;
    ant_array_put(socks, connfd, &info);

    event.data.fd = connfd;
    event.events  = EPOLLIN | EPOLLOUT | EPOLLET;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
        perror("epoll_ctl");
        return -1;
    }
    return 0;
}

static int do_error(struct http_info *http, int efd, ant_array_t *socks) {
    /* record error log*/
    ant_log_err("%s\n", http->down->url);

    close(http->fd);
    ant_array_clear(socks, http->fd);
    http->fd = -1;
    http->try_num++;
    http->status = 0;
    if (http->try_num > 2) {
        ant_debug_sock_url(http->fd, http->down->url);
        exit(1);
    }
    return do_create(http, efd, socks);
}

static int do_write(struct http_info *http, int efd, ant_array_t *socks) {

    struct epoll_event event;
    if(http_send_head(http) == -1) {
        ant_debug_sock_url(http->fd, http->down->url);
        do_error(http, efd, socks);
    }

    event.data.fd = http->fd;
    event.events = EPOLLIN;

    if (epoll_ctl(efd, EPOLL_CTL_MOD, http->fd, &event) < 0) {
        perror("epoll_ctl");
        exit(1);
    }
}

static int do_read(struct http_info *http, char *url_buf, total_context_t *total) {

    http_read_head(http);

    if (http->http_code == 200) {

        ant_log_err("http_read_body begin\n");
        if(http_read_body(http) < 0);

        ant_log_err("http_read_body end\n");

        if (http->status == ANT_DUMP) {
            html_dump(http, total);
        }

    } else if (http->http_code == 301 || http->http_code == 302) {
        if (http->location[0] == '/') {
            char *p = http->down->url + skip_head(http->down->url);
            snprintf(url_buf, MAX_URL_SIZE, "%.*s%s", 
                    (int)strcspn(p, "/?;#"), p, 
                    http->location);
            filter_add(url_buf, total);
        }
        else {
            filter_add(http->location, total);
        }
    }
    else if (http->http_code != 403){
        fprintf(stderr, "http_code = %d\n", http->http_code);
    }
}

int http_event_loop(total_context_t *total) {
    down_t        *down;
    int    efd;
    struct http_info *info = NULL;
    struct http_info *http = NULL;
    struct epoll_event event;
    struct epoll_event *events = NULL;
    /*char buf[1024] = "";*/
    ant_array_t *socks = NULL;

    if ((efd = epoll_create1(0)) < 0)
        ant_error("epoll_create1");

    const ANT_MAX_EVENTS = opt.maxconn;
    events = calloc(ANT_MAX_EVENTS, sizeof(struct epoll_event));
    if (!events) {
        perror("calloc");
        goto fail;
    }

    int i, n;

    socks = ant_array_new(50, sizeof(struct http_info *));
    int ntask   = 0; /* the number of tasks */
    int nconn   = 0; /* is then number of connection*/
    struct sockaddr_in peer;
    char url_buf[MAX_URL_SIZE] = "";
    do {

        for(ntask = 16; ntask > 0 && nconn < ANT_MAX_EVENTS;/*space*/) {
            pthread_mutex_lock(&total->mutex[ANT_DOWN]);
            if(queue_empty(total->down)) {
                pthread_mutex_unlock(&total->mutex[ANT_DOWN]);
                break;
            }

            fprintf(stderr, "nconn = %d:size = %d\n", nconn, total->down->size);
            down = queue_get(total->down);

            status.queue_num = total->down->size;
            pthread_mutex_unlock(&total->mutex[ANT_DOWN]);

            ant_log_url_all("%s\n", down->url);
            status.should_down++;
            ntask--;
            nconn++;
            /* Remove duplicate */
            info = http_new(down);

            if(do_create(info, efd, socks) == -1)
                goto fail;
        } 

        n = epoll_wait(efd, events, ANT_MAX_EVENTS, 1000);
        ant_log_err("n = %d:nconn = %d\n", n, nconn);
        for (i = 0; i < n; i++) {

            http = *(struct http_info **)ant_array_get(socks, events[i].data.fd);
            if (http->fd != events[i].data.fd) {
                fprintf(stderr, "http->fd:%d != event[i].data.fd%d\n",
                        http->fd, events[i].data.fd);
                exit(1);
            }

            if ((events[i].events & EPOLLERR) ||
                    (events[i].events  & EPOLLHUP)
               ) {

                fprintf(stderr, "epoll error\n");

                if (do_error(http, efd, socks) == -1) 
                    goto fail;

            } else {

                /* if then url exists
                 * free http
                 * close sock fd*/
                if(ant_lookup(total->cache_url, http->down->url, NULL)) {
                    status.should_down--;
                    goto clean;
                }

                /* socket error */
                if ((events[i].events & EPOLLOUT)
                        && (events[i].events & EPOLLIN)) {

                    ant_log_err("EPOLLOUT EPOLLIN: %d\n", http_read_head(http));
                    do_error(http, efd, socks);
                }/* socket write */
                else if (events[i].events & EPOLLOUT) {
                    do_write(http, efd, socks);
                }/* socket read */
                else if (events[i].events & EPOLLIN) {

                    do_read(http, url_buf, total);

                    if (http->status == ANT_DUMP || http->http_code != 200) {
                        ant_log_err("================\n");
                        ant_array_clear(socks, events[i].data.fd);
                        /*
                         *url inserl hash url
                         */
                        ant_hash_put(total->cache_url, http->down->url, NULL);
                        http->down->url = NULL;

clean:
                        http_clean(http);
                        nconn--;
                    }
                    ant_log_err("in end\n");
                }
            }
        }

    } while(1);

fail:
    close(efd);
    free(events);
    ant_array_free(socks, NULL);
    return -1;
}

int file_extract_url(char *fname, total_context_t *total) {
    char line[1024];
    char *p;
    FILE *fp;

    fp = fopen(fname, "r");
    if (!fp) {
        fprintf(stderr, "%s:fopen:%m\n", __FUNCTION__);
        return -1;
    }

    while (fgets(line, sizeof line, fp)) {

        /*skip space line*/
        if (ISSPACE_LINE(line))
            continue;

        p = line;
        while (*p && !isspace(*p)) p++;

        if (p) {
            *p = '\0';
            /*fprintf(stderr, "%s\n", line);*/
            filter_add(line, total);
            /*p+1 is file name*/

        }
    }
    fclose(fp);
    return 0;
}

struct http_info *http_new(down_t *down) {
    struct http_info *newp = malloc(sizeof(struct http_info));
    http_init(newp, down);
    return newp;
}

void http_init(struct http_info *info, down_t *down) {
    if (!info)
        return ;
    memset(info, '\0', sizeof(*info) );
    info->fd = -1;

    /* html page */
    info->page = ant_buf_new(0);

    info->down = down;
}

void http_clean(struct http_info *http) {
    free(http->down->url);
    free(http->down->fname);
    free(http->cookies);
    /* not release ip*/

    close(http->fd);
    http->fd = -1;

    if (http->page)
        ant_buf_free(http->page);

    http->page = NULL;

    free(http);
}

static int page_to_utf8(ant_buf_t *dst, ant_buf_t *src, const char *fromcode) {
    int     rc;
    iconv_t cd;
    size_t  nconv;
    char    buf[BUFSIZ] = "";
    char    *inbuf, *outbuf;
    size_t  inlen, outlen;

    rc = 0;
    inbuf  = src->buf;
    outbuf = buf;
    inlen  = src->ncurr;

    cd = iconv_open("utf-8", fromcode);
    if (cd < 0)
        ant_error("iconv_open");

    while (inlen > 0) {

        outbuf = buf;
        outlen = BUFSIZ;
        nconv = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
        if ((nconv == -1 && errno != E2BIG)||
                (BUFSIZ - outlen < 0)) {

            if (nconv == -1)
                perror("iconv");
            rc = -1;
            break;
        }

        ant_buf_append(dst, buf, BUFSIZ - outlen);
    }

    iconv_close(cd);
    return rc;
}


int html_dump(struct http_info *info, total_context_t *total) {
    int       fd, flag, ret, len;
    ant_buf_t *out;
    char code[37] = "";

    ret = 0;
    out = NULL;
    flag = 0;
    fd = -1;

    char *buf;
    if (chdir(DUMP_DIR) == -1) {
        perror("html_dump -> chdir");
        ret = -1;
        goto fail;
    }

    if (info->http_code != 200) {
        ret = -1;
        goto clean;
    }

    setfilename(info);
    assert(access(info->fi.filename, F_OK) != 0);

    /*
     *O_TRUNC must be added
     * */
    fd = open(info->fi.filename, O_RDWR |O_CREAT |O_TRUNC, 0644);
    if (fd == -1) {
        fprintf(stderr, "open %s %m\n", info->fi.filename);
        ret = -1;
        goto clean;
    }

    if ((info->filesize == -1 && info->chunksize == 0)
            || (info->filesize > 0)
            || (info->filesize == -1 && info->chunksize == -1)
       ) {

        /* Determine whether it is utf-8 encoding */
        if (file_looks_utf8(info->page->buf, info->page->ncurr) > 0) {
            flag = -1;
        } else {

            out = ant_buf_new(0);
            flag = extract_fromcode(code, info->page->buf);
            if (flag == 0) {
                if(!strncasecmp(code, "gb", 2))
                    strcpy(code, "gbk");

                /* page code to utf8 */
                fprintf(stderr, "----------from code = %s\n", code);
                flag = page_to_utf8(out, info->page, code);
            }
        }

        int length;
        if (flag) {
            buf = info->page->buf;
            len = info->page->ncurr;
            length = info->page->length;
        } else {
            buf = out->buf;
            len = out->ncurr;
            length = info->page->length;
        }
        write(fd, buf, len);
        fprintf(stderr, "length = %d:cur_len = %d:nm_alloc = %d\n:%s\n", 
                length, len, info->page->nm_alloc, info->down->url);
        if (opt.debug == 0) {
            parse_page(buf, total, filter_add);
        }

        /* record downloaded url*/
        ant_log_url("%s\n", info->down->url);

        status.total_down++;
    } else {
        fprintf(stderr, "filesize = %ld: chunksize = %ld:in %s\n",
                info->filesize, info->chunksize, __FUNCTION__);
    } 


clean:
    chdir("..");
    close(fd);

fail:
    if (out)
        ant_buf_free(out);

    ant_buf_free(info->page);
    info->page = NULL;
    return ret;
}

int down_add(unsigned *ip, unsigned short port, char *url, struct queue_head *queue) {
    down_t *down;
    char   *p;

    down = (down_t *)calloc(1, sizeof(down_t));

    if(!down) {
        perror("malloc");
        return -1;
    }

    down->url    = strdup(url);
    down->fname  = NULL;

    /* ip */
    if (*ip == 0) {
        down->ip  = ip;
    }
    else {
        down->ip2 = *ip;
        down->ip  = NULL;
    }

    /* port */
    down->port = port;
    if (port == 0) {
        p = url + skip_head(url);
        sscanf(p + strcspn(p, ":/"), ":%hu", &down->port);
        fprintf(stderr, "::port = %hu\n", down->port);
    }
    queue_put(down, queue);
    return 0;
}

void filter_add(char *url, total_context_t *total) {

    int rc;
    uint32_t ip;
    char *p = (char *)&ip;
    uint16_t port;

    /*Check the url exists*/
    if (uniq_exists(url) == 0)
        uniq_put(url);
    else 
        return ;

    /* for example:
     * if the url is www.baidu.com then 
     * call url_add 
     * else the url is 119.75.218.77 */
    rc = sscanf(url + skip_head(url),
            "%hhu.%hhu.%hhu.%hhu:%hu", 
            p, p+1, p+2, p+3, &port);

    if (rc == 4 || rc == 5) {
        /*fprintf(stderr, "22222#%d;%s\n", rc, url);*/
        pthread_mutex_lock(&total->mutex[ANT_DOWN]);
        if (rc == 4)
            port = 0;

        down_add(&ip, port, url, total->down);
        pthread_mutex_unlock(&total->mutex[ANT_DOWN]);

    } else {
        /*fprintf(stderr, "11111#%d:%s\n", rc, url);*/
        url_add(url, total);
    }

}

static int read_config(char *fname) {
    FILE *fp;
    char line[1000];
    char *p;

    fp = fopen(fname, "r");
    if (!fp) {
        fprintf(stderr, "%m:fopen [%s] error in %s\n",
                fname, __FUNCTION__);
        return -1;
    }

    int rc;
    while (fgets(line, sizeof(line), fp)) {
        p = line;
        while (isspace(*p))
            p++;

        if(!strncmp(p, "maxconn", 7)) {
            rc = sscanf(p + 7, "%*[ =]%d", &opt.maxconn);
        }
    }

    fclose(fp);
    return 0;
}

void url_add(char *url2, total_context_t *total) {

    char domain[512];
    char url_buf[MAX_URL_SIZE] = "";
    char *url = url2;
    struct cache_val *val;
    ant_hash_elem_t *elem;

    url_to_domain(domain, sizeof(domain), url);

    if (url_modify(url_buf, MAX_URL_SIZE, url, strlen(url)) == 0)
        url = url_buf;

    /* create new dns val*/
    if(!(elem = ant_lookup(total->cache_dns, domain, NULL))) {

        /* insert dns hash 
         * insert domain queue*/
        val = calloc(1, sizeof(struct cache_val));
        val->flags = 0;
        val->ctime = time(NULL);
        val->mtime = time(NULL);
        val->down = queue_new();
        down_add(val->ip, 0, url, val->down);

        /*lock cache_dns*/
        pthread_mutex_lock(&total->mutex[ANT_CACHE_DNS]);
        ant_hash_put(total->cache_dns, strdup(domain), val);
        /*unlock cache_dns*/
        pthread_mutex_unlock(&total->mutex[ANT_CACHE_DNS]);

        /*lock queue_dns*/
        pthread_mutex_lock(&total->mutex[ANT_QUEUE_DNS]);
        queue_put(strdup(domain), total->queue_dns);
        /*unlock queue_dns*/
        pthread_mutex_unlock(&total->mutex[ANT_QUEUE_DNS]);

    } else {

        /* if cache_val->falgs is 1
         * insert val to  total_context_t  down
         * else url append cache_url->val
         * */
        val = elem->val;

        if (!val) {
            fprintf(stderr, "cache_val is null\n");
            return ;
        }

        if (val->flags == 1) {

            pthread_mutex_lock(&total->mutex[ANT_DOWN]);
            down_add(val->ip, 0, url, total->down);
            pthread_mutex_unlock(&total->mutex[ANT_DOWN]);
        } else {
            pthread_mutex_lock(&total->mutex[ANT_ITEM_DOWN]);
            down_add(val->ip, 0, url, val->down);
            pthread_mutex_unlock(&total->mutex[ANT_ITEM_DOWN]);
        }
    }
}

