#include "common.h"

static int process_line(http_context *http, char *line, int line_count);

http_context *http_new(
        int fd, 

        int (*list_dir)(struct http_context *http),

        ssize_t (*tor_read )(int fd, void *buf, size_t len, int flags),
        ssize_t (*tor_write)(int fd, const void *buf, size_t len, int flags)
        ) {
    http_context *http = calloc(1, sizeof(http_context));

    http->fd       = fd;
    snprintf(http->fname, TOR_PATH, "%s", config.data_dir);
    http->debug    = 1;
    http->list_dir = list_dir;
    http->read     = (tor_read  == NULL) ? recv: tor_read;
    http->write    = (tor_write == NULL) ? send: tor_write;

    return http;
}

void http_free(http_context *http) {
    close(http->fd);

    /* free http->page */
    if (http->mm) 
        munmap(http->page, http->puse);
    else
        free(http->page);

    free(http->head);
    free(http);
}

static void http_record_err(char *fmt, ...) {
    va_list args;
    static char buf[TOR_BUFSIZE] = "";
    memset(buf, '\0', TOR_BUFSIZE);
    va_start(args, fmt);
    vsnprintf(buf, TOR_BUFSIZE, fmt, args);
    va_end(args);

    fprintf(stderr, "%s", buf);
}

int http_read_head(http_context *http) {
    char http_buf[TOR_BUFSIZE] = "";
    int line_count = 0;
    char *pos;
    int err;

    assert(http->read != NULL);

    /* check http->record_err */
    if (!(http->record_err))
        http->record_err = http_record_err;

    err = http->read(http->fd, http_buf + http->rblen, 
            TOR_BUFSIZE - http->rblen, MSG_PEEK);

    if ((err < 0) ||
        (err >= 0 && !(pos = strstr(http_buf, "\r\n\r\n")))) {

        if (err > 0)
            http->rblen += err;

        if (!pos) {
            http->record_err("pos is null in %s:%d\n",
                    __FUNCTION__, __LINE__);
        }
        return -1;
    }


    err = http->read(http->fd, http_buf, pos + 4 - http_buf, 0);
    http_buf[err] = '\0';
    /* debug */
    if (http->debug == 1) {
        http->record_err("%s",http_buf);
    }

    char *p = http_buf;
    char *q;

    for (;;) {

        q = p;
        if (*q == '\n') {
            q++;
            p++;
        }
        for (/*space*/;*p; p++) {
            if (*p == '\n') {
                if (p[-1] == '\r')
                    p--;
                *p = '\0';
                p++;
                break;
            }
        }
        err = process_line(http, q, line_count);
        if (err < 0)
            return err;
        if (err == 0)
            break;
        line_count++;
    }

    return 0;
}

static int process_line(http_context *http, char *line, int line_count) {

    char *p, *tag;
    if (line[0] == '\0')
        return 0;

    int len, n;
    if (line_count == 0) {
        int rc;
        if (!strncasecmp(line, "get", 3))
            len = 3;
        rc = sscanf(line + len, "%*[ ]/%3800[^ ]", http->path);
        if (rc != 1) {
            /* record error*/
            http->record_err("path = %s\n", http->path);
            return -1;
        }
    } else {
        p = line;

        while (*p && *p != ':')
            p++;

        if (*p != ':')
            return 1;

        *p = '\0';
        p++;

        tag = line;

        while (isspace(*p))
            p++;

        if (!strcasecmp(tag, "Connection")) {
            if(!strcasecmp(p, "keep-alive"))
                http->conn = 1;
        } else if (!strcasecmp(tag, "Referer")) {
            if (!strncasecmp(p, "http://", 7))
                p += 7;

            p = strchr(p, '/');

            /*
             * if http->fname = ./
             * Referer =        /dir 
             * join http->fname and Referer = .//dir 
             *
             * if the http->fname last char is '/' 
             * Referer skip '/'
             * join http->fname and Referer = ./dir 
             */

            if (p) {
                if (http->fname[strlen(http->fname) - 1] == '/')
                    ++p;

                len = strlen(http->fname);

                /*join http->fname and Referer value*/
                n = snprintf(http->fname + len, MAX_URL_SIZE - len, 
                        "%s", p);
                if (n > -1 && n < MAX_URL_SIZE - len) {

                    /* if the http->fname is dir and last char no '/'*/
                    if (IS_DIR(http->fname) && http->fname[len + n - 1] != '/') {
                        http->fname[len + n] = '/';
                        http->fname[len + n + 1] = '\0';
                    }
                }
            }

#if 0
            printf("%p:http->path = %s\n", p, http->fname);
            printf("Referer = %s\n", p);
#endif
        }
    }

    return 1;
}

int http_join_head_body(http_context *http) {
    struct stat sb;
    int r, fd;

    int code = 200;
    char *msg = "OK";

    fd = -1;
    
    strncat(http->fname, http->path, 
            MAX_URL_SIZE - strlen(http->fname) - strlen(http->path) - 1);

    r = stat(http->fname, &sb);
    fprintf(stderr, "%d:%s:%s\n", r, http->path, http->fname);

    if (r == 0) {
        /* determine the file type */
        if (S_ISDIR(sb.st_mode)) {
            r = http->list_dir(http);
            printf("---------dir:%d\n", r);
        } else if (S_ISREG(sb.st_mode)) {
            http->mm = 1;
            fd = open(http->fname, O_RDONLY, 0644);
            if (fd != -1) {
                http->page = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
                http->puse = sb.st_size;
                printf("::::::%ld:%d\n", (long)http->page, http->puse);
            }
        }
    } 

    printf("--------fd = %d\n", fd);
    if (errno == ENOENT) {
        code = 404;
        msg = "Not Found";
    }

    sprintfcat(&http->head, &http->huse, &http->hsize,
            "HTTP/1.1 %d %s\r\n", code, msg);
    sprintfcat(&http->head, &http->huse, &http->hsize,
            "\r\n");
    close(fd);

    /*print head*/
    write(1, http->head, http->huse);
    /*print body*/
    write(1, http->page, http->puse);
}

int http_write(http_context *http) {
    struct iovec iov[2];

    iov[0].iov_base = http->head;
    iov[0].iov_len  = http->huse;

    iov[1].iov_base = http->page;
    iov[1].iov_len  = http->puse;

    writev(http->fd, iov, (iov[1].iov_len == 0) ? 1 : 2);
}

#ifdef _TOR_TEST
#include <fcntl.h>
int main() {

    http_context *http;
    http = calloc(1, sizeof(http_context));

    int fd;
    fd = open("test.data", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("");
        exit(1);
    }
    http->fd = fd;
    http->debug = 1;
    http->record_err = http_record_err;
    http->read = read;
    int rc;
    for (;;) {
        rc = http_read_head(http);
        fprintf(stderr, "-----::%d---------\n", rc);
        sleep(1);
    }
    close(fd);
    return 0;
}
#endif
