#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "ant_dns.h"
#include "ant_queue.h"
#include "ant_socket.h"
#include "ant_context.h"

static int dns_recv_packet(int sock, dns_response_t *response);

static void set_qname(const char *s, char *buf);

static int set_dns_packets(struct dns_header *dhp, struct dns_question *dqp, 
        const char *name, char *packet, int id);

static int parse_dns_packets(const char *buf, struct dns_header *dhp, dns_response_t *data);


static int dns_send_packet(int sock, char *name);

#ifdef _ANT_TEST
int main(int argc, char **argv) {

    if (argc != 2) {
        printf("<a.out> <text name>\n");
        return -1;
    }

    pthread_t dns_id;
    int rc = 0;
    /* dns main */
    rc = pthread_create(&dns_id, NULL, thread_dns_main, argv[1]);
    if (rc != 0) {
        perror("pthread_create");
    }
    pthread_join(dns_id, NULL);
    printf("join\n");
    return 0;
}
#endif

static
int set_dns_packets(struct dns_header *dhp, struct dns_question *dqp, 
        const char *name, char *packet, int id) {

    memset(dhp, '\0', sizeof(struct dns_header));

    dhp->id = htons(id);
    dhp->flags = htons(0x0100); 
    dhp->questions = htons(0x0001);

    memset(dqp, '\0', sizeof(struct dns_question));

    /*1 is ipv4 address*/
    dqp->qtype = htons(0x0001); 
    /*1 is internet data*/
    dqp->qclass = htons(0x0001);

    /* why qnlen + 2
     * 1 is '\0'
     * for example 
     * 1 is here
     * the www.baidu.com length is 14
     * the 3www5baidu3com length is 15
     * */
    int qnlen = strlen(name)+2;
    char *qnbuf = malloc(qnlen);
    set_qname(name, qnbuf);

    char *bp = packet;
    memcpy(bp, dhp, sizeof(struct dns_header));
    bp += sizeof(struct dns_header);

    memcpy(bp, qnbuf, qnlen);
    bp += qnlen;

    memcpy(bp, &dqp->qtype, sizeof(dqp->qtype));
    bp += sizeof(dqp->qtype);

    memcpy(bp, &dqp->qclass, sizeof(dqp->qclass));
    bp += sizeof(dqp->qclass);

    free(qnbuf);
    return bp - packet;
}

static
int parse_dns_packets(const char *buf, struct dns_header *dhp, dns_response_t *data) {
    char        *bp;
    char        *p;
    int         i, len, ipl;
    uint16_t    type;
    int         num;

    bp = (char *)buf;
    memset(dhp, '\0', sizeof(struct dns_header));
    dhp->id = ntohs(*(uint16_t*)bp);
    /*skip id*/
    bp += 2; 

    dhp->flags = ntohs(*(uint16_t*)bp);
    data->flags = dhp->flags;
    /*skip flags*/
    bp += 2;

    /*skip questions*/
    bp += 2;
    dhp->answers_rrs = ntohs(*(uint16_t*)bp);


    /*skip answers_rrs    16 bit
      skip authority_rrs  16 bit
      skip additional_rrs 16 bit
      */ 
    bp += 6;

    /* extract host name */
    for (p = data->host_name; *bp; /*space*/) {
        num = *bp++;
        while (num-- > 0)
            *p++ = *bp++;
        *p++ = '.';
    }
    *--p = '\0';

    if (dhp->flags != 0x8180) {
        fprintf(stderr, "dhp->flags != 0x8180:%x:%s\n", dhp->flags, data->host_name);
        ant_log_err("dhp->flags != 0x8180:%x:%s:\n", dhp->flags, data->host_name);
        return -1;
    }

    bp += 1; /* skip 0 */
    bp += 2; /* skip type*/
    bp += 2; /* skip class*/

    num = 0;
    for (i = 0, len = dhp->answers_rrs; i < len; i++) {
        /* name */
        bp += 2;

        /* type is 16 bit*/
        type = ntohs(*(uint16_t*)bp);
        bp += 2;

        /*class is 16 bit
         * ttl is 32 bit  */
        bp += 6;

        ipl = ntohs(*(uint16_t*)bp);
        bp += 2;

        if (type == 0x0001 && ipl == 4) {
            if (num < IP_LEN)
                /*data->ip[num++] = ntohl(*(uint32_t*)bp);*/
                data->ip[num++] = *(uint32_t*)bp;
            else 
                return 0;
            //printf("%d:%d:%d:%d\n", (ip >> 24) & 0xff,(ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
        }
        bp += ipl;
    }
    return 0;
}

static 
void set_qname(const char *s, char *buf) {
    char *curr ,*prev;
    char *bp = buf;
    int  cnt = 0;

    /* for example
     * domain name: www.baidu.com ==> 3www5baidu3com0
     * 3,5,3is lenght
     * */
    curr = prev = (char*)s;
    for(; *curr; curr++) {
        if (*curr == '.') {
            /* cnt is the string length
             * for example 
             * first cnt is 3
             * two cnt is 5
             * */
            cnt = curr - prev;
            *bp++ = cnt;

            /* for example
             * first:       two:
             * prev => w    prev => b
             * curr => .    curr => .
             * */
            memcpy(bp, prev, cnt);
            bp += cnt;

            /* why curr + 1
             * skip '.'
             * */

            prev = curr+1;
        }

    }

    cnt = curr - prev;
    *bp++ = cnt;
    memcpy(bp, prev, cnt);
    bp[cnt] = 0;
}

static 
int dns_recv_packet(int sock, dns_response_t *response) {
    uint16_t            head;
    int                 rc;
    char                res[1024];
    struct dns_header   dheader;

    /* read header*/
    rc = readn_block(sock, &head, 2);
    if (rc <= 0)
        return rc;
    head = ntohs(head);

    //printf("::%d\n", head);

    if (head > sizeof(res)) {
        /*record log*/
        fprintf(stderr, "in %s head = %d\n", __FUNCTION__, head);
        return -1;
    }

    /* read body */
    rc = readn_block(sock, res, head);

    parse_dns_packets(res, &dheader, response);
    return rc;
}

static 
int dns_send_packet(int sock, char *name) {
    struct dns_header dh;
    struct dns_question dq;
    int palen;
    char send_buf[512];
    static unsigned id = 0;

    palen = set_dns_packets(&dh, &dq, name, send_buf + 2, id++);
    *(uint16_t*)send_buf = htons(palen);
    return send(sock, send_buf, palen + 2, 0);
}

/* debug */
static void print_response (dns_response_t *response) {
    int i;
    unsigned ip;
    for (i = 0; i < IP_LEN; i++) {
        ip = response->ip[i];
        if (ip == 0)
            break;
        fprintf(stderr, "%s#%d:%d:%d:%d\n",
                response->host_name, ip & 0xff , (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    }
}

#define MAX_NCONN  2
#define DNS_MAX_EVENT  4

static void clean_fd(int sock, int *cache_sock, int *nsock) {
    int i;
    for (i = 0; i < MAX_NCONN; i++) {
        if (cache_sock[i] == sock)
            cache_sock[i] = 0;
    }

    close(sock);
    (*nsock)--;
}

static void process_cache_dns(total_context_t *total, dns_response_t response) {
    struct cache_val *val;

    pthread_mutex_lock(&total->mutex[ANT_CACHE_DNS]);

    val = ant_hash_get(total->cache_dns, response.host_name);

    if (val) {
        /*val->flags is 1 
         * dns hash with the domain name, ip
         * */
        if (val->flags == 1)
            goto done ;

        if (response.flags != 0x8180) {
            ant_log_err("dns parse err %s\n", response.host_name);;
            goto done;
        }

        memcpy(val->ip, response.ip, sizeof(response.ip));
        val->mtime = time(NULL);
        val->flags = 1;

        pthread_mutex_lock(&total->mutex[ANT_DOWN]);
#if 0
        fprintf(stderr, "%s\n", response.host_name);
        fprintf(stderr, "%d:%p:%p:%p",
                total->down->size, total->down, total->down->head, total->down->tail);
        fprintf(stderr, "----------%p\n",
                val->down);
                /*val->down->size,val->down, val->down->head, val->down->tail);*/
#endif
        queue_join_init(total->down, val->down);
        queue_free(val->down);
        val->down = NULL;
        pthread_mutex_unlock(&total->mutex[ANT_DOWN]);
    } else {
        fprintf(stderr, "val is NULL\n");
    }

done:
    pthread_mutex_unlock(&total->mutex[ANT_CACHE_DNS]);
}

static int get_dns_ip(char *buf, size_t n) {
    FILE *fp;
    char line[512];
    char *pos, *p;

    fp = NULL;
    pos= NULL;
    fp = fopen("/etc/resolv.conf", "r");

    if (!fp) {
        perror("fopen /etc/resolv.conf");
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#')
            continue;

        if ((pos = strchr(line, '\n')))
            *pos = '\0';
        
        p = line;
        while (*p && !isspace(*p)) p++;

        if (!isspace(*p))
            continue;

        *p = '\0';
        p++;

        if (!strcmp(line, "nameserver")) {
            if (!strcmp(p, "127.0.0.1"))
                continue;
        }
        else if (!strcmp(line, "search")) {
            if (inet_addr(p) == -1)
                continue;
        }
        else  {
            continue;
        }

        strncpy(buf, p, n);
        if (n > 0)
            buf[n - 1] = '\0';

        fclose(fp);
        return 0;
    }

    fclose(fp);
    return -1;
}

void *thread_dns_main(void *argv) {
    total_context_t *total;

    total = (total_context_t *)argv;
    /*struct queue_head *qh;*/

    /* dns server address*/
    char buf[512] = "";
    if(get_dns_ip(buf, sizeof(buf)) == -1) {
        fprintf(stderr, "/ect/resolv.conf"
                " dns server address not available\n");
        exit(EXIT_FAILURE);
    }
    unsigned ip = inet_addr(buf);
    struct sockaddr_in addr;
    int rc = 0;
    char *name = NULL;

    int check = 1;
    int sock, efd;
    int i, n, ntask;
    int timeout = 2000;
    struct epoll_event event = {0};
    struct epoll_event *events = NULL;
    int cache_sock[MAX_NCONN] = {0};
    dns_response_t response;

    if ((efd = epoll_create1(0)) < 0) {
        perror("epoll_create1");
        goto fail;
    }

    events = calloc(DNS_MAX_EVENT, sizeof(struct epoll_event));

    if (!events) {
        perror("calloc");
        goto fail;
    }

    int nsock = 0;

    for (;;) {

        /* nsock :: The number of the sock being used
         * create connect */
        while (nsock < MAX_NCONN) {
            sock = sock_nonblock_create(&addr, ip, 53);
            if (sock < 0) {
                perror("create sock");
                goto fail;
            }

            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                if (errno != EINPROGRESS) {
                    perror("connect");
                    goto fail;
                }

            event.data.fd = sock;
            event.events = EPOLLIN | EPOLLOUT;

            if (epoll_ctl(efd, EPOLL_CTL_ADD, sock, &event) < 0) {
                perror("dns epoll_ctl");
                goto fail;
            }

            nsock++;
        }

        n = epoll_wait(efd, events, DNS_MAX_EVENT, timeout);

        if (check && n == 0) {
            fprintf(stderr, "dns server connection timed out\n");
            exit(0);
        }
        for (i = 0; i < n; i++) {

            sock = events[i].data.fd;
            if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP)
               ) {

                fprintf(stderr, "dns epoll error\n");
                clean_fd(sock, cache_sock, &nsock);
                continue;

            } else {

                check = 0;
                memset(&response, '\0', sizeof(response));

                if (events[i].events & EPOLLIN) {
                    rc = dns_recv_packet(sock, &response);
                    if ((rc == 0) || (rc == -1 && errno != EAGAIN)) {
                        fprintf(stderr, "dns_recv_packet = %d----close\n",rc);

                        clean_fd(sock, cache_sock, &nsock);
                        continue;
                    }

                    /* debug */
                    print_response(&response);
                    process_cache_dns(total, response);
                }

                if (events[i].events & EPOLLOUT) {

                    int j;
                    for (j = 0; j < MAX_NCONN; j++)
                        if (cache_sock[j] == 0) {
                            cache_sock[j] = sock;
                            break;
                        }

                    /*修改 event 由EPOLLIN| EPOLLOUT 变为 EPOLLIN
                     * 如果同时注册EPOLLIN |EPOLLOUT, epoll_wait 几乎是立即返回,效率不给力
                     * 前面注册EPOLLOUT 是为了监听 异步的connect 链接成功没
                     * */
                    /*Modify the event by the EPOLLIN | EPOLLOUT becomes EPOLLIN
                     *If both registered EPOLLIN | EPOLLOUT, 
                     * epoll_wait almost immediately returned, the efficiency is not to force
                     *Sign EPOLLOUT front asynchronous connect to monitor the success did not link
                     */
                    event.data.fd = sock;
                    event.events = EPOLLIN;

                    if (epoll_ctl(efd, EPOLL_CTL_MOD, sock, &event) < 0) {
                        perror("dns epoll_ctl");
                        goto fail;
                    }
                }
            }
        }

        /* 可使用的sock 存放在cache_sock中
         * 为什么要自己维护可使用的sock,是为了应付一种概率不高的情况
         * 常规写法:一开始epoll_wait 无限阻塞,EPOLLOUT 写数据,EPOLLIN 读数据
         * 在客服端这么使用的话会出现尴尬,EPOLLOUT中发送的请求包,正好在EPOLLINT解完
         * epoll_wait 一直在等EPOLLIN事件,关键是你不发请求就没有响应就没有EPOLLIN事件
         * 那如果注册的时候选EPOLLIN|EPOLLOUT,效率不给力,epoll_wait几乎是立即返回,等同空转
         * */
        /* Can be used in the sock stored in cache_sock
         * Why should maintain their own use of sock, is to respond to a probability is not high
         * Conventional wording: a start epoll_wait unlimited blocking, EPOLLOUT write data, EPOLLIN read data
         * In the customer service side so it will be awkward to use,
         * EPOLLOUT sent in the request packet, just in EPOLLINT end solution.
         * Epoll_wait been waiting EPOLLIN event, 
         * the key is that you do not send the request would not respond no EPOLLIN event.
         *That if the registration of the election EPOLLIN | EPOLLOUT, 
         *efficiency is not to force, epoll_wait almost immediately returned, equivalent to idling.
          */
        for (i = 0; i < MAX_NCONN; i++) {
            if (cache_sock[i] == 0)
                continue;
            
            for (ntask = 4; ntask-- > 0; /*space*/) {

                /* lock begin */
                pthread_mutex_lock(&total->mutex[ANT_QUEUE_DNS]);

                if(queue_empty(total->queue_dns)) {
                    pthread_mutex_unlock(&total->mutex[ANT_QUEUE_DNS]);
                    break;
                }
                name = queue_get(total->queue_dns);

                /* lock end */
                pthread_mutex_unlock(&total->mutex[ANT_QUEUE_DNS]);

                rc = dns_send_packet(cache_sock[i], name);
                free(name);
                if ((rc == -1 && errno != EAGAIN) || rc == 0) {
                    fprintf(stderr, "send rc = %d:cache_sock[%d] = %d\n", rc, i, cache_sock[i]);
                    clean_fd(cache_sock[i], cache_sock, &nsock);
                    break;
                }
            }
        }

    }/* end for(;;) */

fail:
    free(events);
    close(efd);
    return (void *)-1;
}
#undef MAX_NCONN  
#undef DNS_MAX_EVENT  
