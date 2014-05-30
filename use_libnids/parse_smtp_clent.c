#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "nids.h"
#include "ant_queue.h"
#include "ant_hash.h"

typedef struct ant_data_s {
    void *data;
    int len;
} ant_data_t;

#define int_ntoa(x)	inet_ntoa(*((struct in_addr *)&x))

/* Example 2: disabling checksums of packets with src ip of any local interface */
static int get_all_ifaces(struct ifreq **ifreq, int * count);

static unsigned int get_addr_from_ifreq(struct ifreq *ifreq);

int all_local_ipaddrs_chksum_disable();

/* helper functions for Example 2 */
unsigned int get_addr_from_ifreq(struct ifreq *iface);

static int get_all_ifaces(struct ifreq **ifaces, int *count);

char * adres (struct tuple4 addr);

static
char *set_mail_name(char *dst, ant_data_t *data);

int dump_file(struct queue_head *queue, char *fname); 

void ant_smtp_init(struct tcp_stream *a_tcp, struct half_stream *hlf);

void ant_http_init(struct tcp_stream *a_tcp, struct half_stream *hlf);

void tcp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed);

void ant_smtp_process(struct tcp_stream *a_tcp, struct half_stream *hlf);

void ant_http_process(struct tcp_stream *a_tcp, struct half_stream *hlf);

void tcp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed);

enum ANT_DUMP_STATUS{
    ANT_INIT,
    ANT_BEGIN,
    ANT_DOWNLOAD
};

typedef struct ant_status_s {
    enum ANT_DUMP_STATUS smtp;
    enum ANT_DUMP_STATUS http;
} ant_status_t;


ant_hash_t *g_smtp_hash = NULL;
ant_hash_t *g_http_hash = NULL;

ant_status_t g_dump_status = {
    .smtp = ANT_INIT,
    .http = ANT_INIT
};

typedef struct ant_jump_table_s {
    int port;
    void (*func)(struct tcp_stream *, struct half_stream *);
}ant_jump_table_t;

#define ant_count(array) (sizeof(array) / sizeof(array[0]))

ant_jump_table_t jump_pact_init[] = {
    {.port = 25, .func = ant_smtp_init},
    {.port = 80, .func = ant_http_init}
};

ant_jump_table_t jump_pact_process[] = {
    {.port = 25, .func = ant_smtp_process},
    {.port = 80, .func = ant_http_process}
};

void ant_smtp_process(struct tcp_stream *a_tcp, struct half_stream *hlf) {
    char fname[512] = "";
    struct queue_head *queue;

    queue = ant_hash_get(g_smtp_hash, &a_tcp->addr);
    if (g_dump_status.smtp == ANT_BEGIN && hlf->count_new > 0) {

        /* put queue */
        ant_data_t *d = malloc(sizeof(ant_data_t));
        d->data = malloc(hlf->count_new);

        memcpy(d->data, hlf->data, hlf->count_new);
        d->len  = hlf->count_new;
        if (queue) {
            /* here */
            queue_put(d, queue);
        } else {
            free(d->data);
            free(d);
        }
    } else if (g_dump_status.smtp == ANT_DOWNLOAD) {
        /*printf("-----download--------- g_dump_status.smtp  = %d\n", g_dump_status.smtp);*/
        /* set mail file name */
        set_mail_name(fname, queue->head->data);
        printf("fname = %s\n", fname);
        /* dump file*/
        dump_file(queue, fname);
        g_dump_status.smtp = ANT_INIT;

        /* clean */
        ant_hash_del(g_smtp_hash, &a_tcp->addr);
        free(queue);
    }
}

void ant_smtp_init(struct tcp_stream *a_tcp, struct half_stream *hlf) {
    struct queue_head *queue;
    struct tuple4 *ptr;
    if (!strncmp(hlf->data, "Return-Path:", sizeof("Return-Path:") - 1)) {
        g_dump_status.smtp = ANT_BEGIN;
        printf("--------------- g_dump_status.smtp  = %d\n", g_dump_status.smtp);

        /* init hash key */
        ptr = malloc(sizeof(struct tuple4));
        memcpy(ptr, &a_tcp->addr, sizeof(struct tuple4));

        /* queue is hash val */
        queue = queue_new();
        if (queue == NULL) {
            fprintf(stderr, "log: queue create is NULL\n");
        }
        /* ant_hash_put(hash, key, val)*/
        ant_hash_put(g_smtp_hash, ptr, queue);
        /*printf("ant_hash_get %x\n", (unsigned int )ant_hash_get(g_smtp_hash, ptr));*/
    } else if (!strncmp(hlf->data, "QUIT", sizeof("QUIT") - 1)) {
        g_dump_status.smtp = ANT_DOWNLOAD;
    }
}

void ant_http_init(struct tcp_stream *a_tcp, struct half_stream *hlf) {
    printf("hello http_init\n");
}

void ant_http_process(struct tcp_stream *a_tcp, struct half_stream *hlf) {
    printf("hello http_process\n");
}


int main ()
{
    g_smtp_hash = ant_hash_new(40, sizeof(struct tuple4), NULL);
    g_http_hash = ant_hash_new(40, sizeof(struct tuple4), NULL);
    all_local_ipaddrs_chksum_disable();

    nids_params.device = "eth0";
    if (!nids_init ()) {
        fprintf(stderr,"%s\n",nids_errbuf);
        exit(1);
    }
    nids_register_tcp (tcp_callback);
    nids_run ();
    return 0;
}

int all_local_ipaddrs_chksum_disable() {
    struct ifreq *ifaces;
    int ifaces_count;
    int i, ind = 0;
    struct nids_chksum_ctl *ctlp;
    unsigned int tmp;

    if (!get_all_ifaces(&ifaces, &ifaces_count))
        return -1;
    ctlp =
        (struct nids_chksum_ctl *) malloc(ifaces_count *
                sizeof(struct
                    nids_chksum_ctl));
    if (!ctlp)
        return -1;
    for (i = 0; i < ifaces_count; i++) {
        tmp = get_addr_from_ifreq(ifaces + i);
        if (tmp) {
            ctlp[ind].netaddr = tmp;
            ctlp[ind].mask = inet_addr("255.255.255.255");
            ctlp[ind].action = NIDS_DONT_CHKSUM;
            ind++;
        }
    }
    free(ifaces);
    nids_register_chksum_ctl(ctlp, ind);
    return 0;
}

unsigned int get_addr_from_ifreq(struct ifreq *iface) {
    if (iface->ifr_addr.sa_family == AF_INET)
        return ((struct sockaddr_in *) &(iface->ifr_addr))->
            sin_addr.s_addr;
    return 0;
}

static int get_all_ifaces(struct ifreq **ifaces, int *count)
{
    int ifaces_size = 8 * sizeof(struct ifreq);
    struct ifconf param;
    int sock;

    *ifaces = malloc(ifaces_size);
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock <= 0)
        return 0;
    for (;;) {
        param.ifc_len = ifaces_size;
        param.ifc_req = *ifaces;
        if (ioctl(sock, SIOCGIFCONF, &param))
            goto err;
        if (param.ifc_len < ifaces_size)
            break;
        free(*ifaces);
        ifaces_size *= 2;
        ifaces = malloc(ifaces_size);
    }
    *count = param.ifc_len / sizeof(struct ifreq);
    close(sock);
    return 1;
err:
    close(sock);
    return 0;
}
char * adres (struct tuple4 addr)
{
    static char buf[256];
    strcpy (buf, int_ntoa (addr.saddr));
    sprintf (buf + strlen (buf), ",%i,", addr.source);
    strcat (buf, int_ntoa (addr.daddr));
    sprintf (buf + strlen (buf), ",%i", addr.dest);
    return buf;
}

static
char *set_mail_name(char *dst, ant_data_t *data) {
    struct timeval tv;
    char *p, *src;
    char buf[256] = "";

    p = dst;
    src = NULL;

    gettimeofday(&tv, NULL);
    sprintf(dst, "%ld", tv.tv_sec);
    p += strlen(dst);

    if ((src = strstr(data->data, "From:")) != NULL) {
        sscanf(src, "From:%[^\r\n]", buf);
        sprintf(p, "-from %s", buf);
        p += strlen(p);
    }

    if ((src = strstr(data->data, "From:")) != NULL) {
        sscanf(src, "To:%[^\r\n]", buf);
        sprintf(p, "-to %s", buf);
    }
    return dst;
}

int dump_file(struct queue_head *queue, char *fname) {
    int fd;
    ant_data_t *pos;

    fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        fprintf(stderr, "dump_file: open %s %m\n", fname);
        return -1;
    }

    while (!queue_empty(queue)) {
        pos = (ant_data_t *)queue_get(queue);
        write(fd, pos->data, pos->len);
        free(pos->data);
        free(pos);
    }
    return 0;
}

void tcp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed) {
    char buf[1024];

    if (a_tcp->addr.dest != 25 && a_tcp->addr.dest != 80)
        return;

    strcpy (buf, adres(a_tcp->addr)); 

    if (a_tcp->nids_state == NIDS_JUST_EST)
    {
        a_tcp->client.collect++; 
        a_tcp->server.collect++; 
        a_tcp->server.collect_urg++; 
        // server
#ifdef WE_WANT_URGENT_DATA_RECEIVED_BY_A_CLIENT
        a_tcp->client.collect_urg++;
        // we won't be notified of urgent data
        // arrival
#endif
        fprintf (stderr, "%s established\n", buf);
        return;
    }
    if (a_tcp->nids_state == NIDS_CLOSE) {
        fprintf (stderr, "%s closing\n", buf);
        return;
    }
    if (a_tcp->nids_state == NIDS_RESET) {
        fprintf (stderr, "%s reset\n", buf);
        return;
    }

    if (a_tcp->nids_state == NIDS_DATA) {

        struct half_stream *hlf;

        if (a_tcp->server.count_new_urg) {
            strcat(buf,"(urgent->)");
            buf[strlen(buf)+1]=0;
            buf[strlen(buf)]=a_tcp->server.urgdata;
            write(1,buf,strlen(buf));
            return;
        }
        if (a_tcp->client.count_new) {
            // new data for client
            hlf = &a_tcp->client; 
            // which will point to client side of conn
            strcat (buf, "(<-)");
        }
        else {
            hlf = &a_tcp->server;
            strcat (buf, "(->)");
        }
#if 0
        fprintf(stderr,"%s",buf);
        write(2,hlf->data,hlf->count_new);
#endif

        /*pack init*/
        int i, len;
        i = 0;
        len = ant_count(jump_pact_init);
        for (/*space*/; i < len; i++) {
            if (a_tcp->addr.dest == jump_pact_init[i].port) {
                jump_pact_init[i].func(a_tcp, hlf);
                break;
            }
        }

        /*pack process*/
        i = 0;
        len = ant_count(jump_pact_process);
        for (/*space*/; i < len; i++) {
            if (a_tcp->addr.dest == jump_pact_process[i].port) {
                jump_pact_process[i].func(a_tcp, hlf);
                break;
            }
        }

    }
    return ;
}
