#include <getopt.h>
#include <sys/epoll.h>
#include "common.h"
#define TOR_MAX_EVENT 256

int do_loop(void);
int read_config(char *fname);

static void usage(char *argv0) {
    fprintf(stderr, "Usage : %s \r\n"
            "\t-c config\r\n", argv0);
}

int main(int argc, char **argv) {
    const char *opstring = "c:";
    const struct option longopts[] = {
        {"config", 1, NULL, 'c'},
    };
    char c;
    char *fname = NULL;
    unsigned int check = 0x0;

    while ((c = getopt_long(argc, argv, opstring, longopts, NULL)) != -1) {
        switch(c) {
            case 'c':
                fname = optarg;
                check = 0x1;
                break;
            default:
                usage(argv[0]);
        }
    }

    if (check == 0x0) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    read_config(fname);
    tor_log_init();
    do_loop();
    fprintf(stderr, "end\n");
    return 0;
}

int read_config(char *fname) {
    FILE *fp;
    char line[1000];
    char *p, *tag, *pos;

    fp = fopen(fname, "r");
    if (!fp) {
        fprintf(stderr, "fopen %sconfig %m\n",
                fname);
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        /*skip " \t\r\n" */
        tag = p = line + strspn(p, " \t\r\n");

        p += strcspn(p, " \t\r\n=");

        if (!*p)
            continue;

        if (isspace(*p)) {
            *p++ = '\0';
            p += strspn(p, " \t\r\n");
        }

        *p++ = '\0';

        p += strspn(p, " \t\r\n");

        /* del \n */
        if ((pos = strchr(p, '\n')) != NULL)
            *pos = '\0';

        if (!strcasecmp(tag, "data_dir")) {
            strncpy(config.data_dir, p, TOR_PATH - 1);
            config.data_dir[TOR_PATH - 1] = '\0';

        } else if (!strcasecmp(tag, "port")) {
            config.port = atoi(p);

        } else if (!strcasecmp(tag, "ip")) {
            char *p = (char *)&config.ip;
            sscanf(p, "%hhu.%hhu.%hhu.%hhu", p, p + 1, p + 2, p + 3);

        } else if (!strcasecmp(tag, "logfile")) {
            snprintf(config.logfile, TOR_PATH, "%s", p);
        }

    }

    fclose(fp);
    return 0;
}

static int fd_add(int efd, int fd) {
    struct epoll_event event;
    int rc;

    event.data.fd = fd;
    event.events = EPOLLIN;

    rc = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
    if (rc < 0) {
        perror("epoll_ctl");
        return -1;
    }

    return 0;
}

int list_dir(http_context *http) {
    DIR *dirp;
    struct dirent *dir;

    if ((dirp = opendir(http->fname)) == NULL) {
        fprintf(stderr, "opendir error %m\n");
        return -1;
    }

    sprintfcat(&http->page, &http->puse, &http->psize, "<html>");
    while ((dir = readdir(dirp)) != NULL) {
        if ((strcmp(dir->d_name, ".") == 0) ||
            (strcmp(dir->d_name, "..") == 0)) {
            continue;
        }
        sprintfcat(&http->page, &http->puse, &http->psize,
                "<a href = %s> %s </a> </br>", 
                dir->d_name, dir->d_name);
    }
    sprintfcat(&http->page, &http->puse, &http->psize, "</html>");
    return 0;
}

void do_process(http_context *http) {
    http_join_head_body(http);
    http_write(http);
}

int do_loop(void) {
    int efd;
    int sfd;
    int rc;
    int connfd;
    struct epoll_event event;
    struct epoll_event *events;

    tor_array_t *cache_sock;
    cache_sock = tor_array_new(TOR_MAX_EVENT, sizeof(http_context *));

    events = calloc(TOR_MAX_EVENT, sizeof(struct epoll_event));
    if (!events) {
        perror("");
        exit(EXIT_FAILURE);
    }


    rc = 0;
    sfd = create_and_bind(INADDR_ANY, config.port);
    if (sfd < 0) {
        exit(EXIT_FAILURE);
    }

    rc = set_fl(sfd, O_NONBLOCK);
    if (rc < 0) {
        exit(EXIT_FAILURE);
    }

    rc = listen(sfd, 10);
    if (rc < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    efd = epoll_create1(0);
    if (efd < 0) {
        perror("");
        exit(EXIT_FAILURE);
    }

    if (fd_add(efd, sfd) < 0)
        exit(EXIT_FAILURE);

    int i, n;
    http_context *http;
    for (;;) {
        n = epoll_wait(efd, events, TOR_MAX_EVENT, -1);
        for (i = 0; i < n; i++) {

            if ((events[i].events & EPOLLERR) || 
                    (events[i].events & EPOLLHUP) ||
                    !(events[i].events & EPOLLIN)) {
                close(events[i].data.fd);
                tor_log_err("epoll error %d\n", events[i].data.fd);

            } else if (events[i].data.fd == sfd) {

                for (;;) {

                    connfd = accept(sfd, NULL, NULL);

                    if (connfd == -1) {
                        if ((errno != EAGAIN) &&
                                (errno != EWOULDBLOCK)) {
                            perror("accept");
                        }
                        break;
                    }

                    if (set_fl(connfd, O_NONBLOCK) < 0) {
                        perror("set_fl");
                        exit(EXIT_FAILURE);
                    }

                    if (fd_add(efd, connfd) < 0) {
                        perror("fd_add");
                        exit(EXIT_FAILURE);
                    }

                    http = http_new(connfd, list_dir, sock_read, writen_nonblock);
                    
                    fprintf(stderr, "put fd = %d:sfd = %d http = %p\n", connfd, sfd, http);
                    tor_array_put(cache_sock, connfd, &http);
                }

            } else {
                fprintf(stderr, "get fd = %d\n", events[i].data.fd);
                http = *(http_context **)tor_array_get(cache_sock, events[i].data.fd);
                fprintf(stderr, "fd = %d\n", http->fd);
                http_read_head(http);
                do_process(http);
                http_free(http);
            }

        }

        fprintf(stderr, "=============\n");
    }

    close(sfd);
    close(efd);
}
