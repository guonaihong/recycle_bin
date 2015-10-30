#include "ant_buf.h"
#include "ant_macro.h"
#include "ant_socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include "ant_http.h"

static int process_line(struct http_info *http, char *line, int line_count);

int http_send_head(struct http_info *http) {
    int fd = http->fd;
    int   len = 0;
    char  *domain = http->down->url;
    char  http_head[4096];
    char  *path;

    if (http->status & ANT_WRITE)
        return 0;

    if (!strncmp( domain, "http://",7) ||!strncmp(domain, "https://", 8)) {
        len = (domain[4] == 's') ? 8 : 7;
    } 

    /* skip head */
    domain += len;

    len = strcspn(domain, "/?#;");
    path = domain + len;

    snprintf(http_head, sizeof(http_head) - 1,
            "GET /%s HTTP/1.1\r\n" 
            "Host: %.*s\r\n" 
            "User-Agent: Mozilla/5.0 Firefox/22.0\r\n" 
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,q=0.8\r\n"
            "DNT: 1\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            ,*path == '/' ? path + 1 : path
            ,len, domain
            );

    fprintf(stderr, "%s\n", http_head );
    if (send(fd, http_head, strlen(http_head), 0) < 0 ) 
        ant_error("send");

    http->status |= ANT_WRITE;
    return 0;
}

int http_read_head(struct http_info *http) {
    int fd = http->fd;
    int err = 0;
    char line[1024];
    char response[BUFSIZ] = "";
    char *pos = NULL, *p = response, *ptr = line;

    if (http->status & ANT_READ_HEAD)
        return 0;

    http->filesize  = -1;
    http->chunksize = -1;

    /* first peek at the available data*/
    if (sock_read(fd, response, sizeof(response), MSG_PEEK) < 0) {
        perror("recv MSG_PEEK error");
        return -1;
    }

    /* "\r\n\r\n" is response header end*/
    if ((pos = strstr(response, "\r\n\r\n"))) {

        memset(response, 0, sizeof(response));
        if(readn_nonblock(fd,response, pos+4-response, NULL) < 0)
            ant_error("readn_nonblock error");

        for (;;) {
            ptr = line;

            /* copy a row of data into the buffer*/
            while ( *p ) {
                if ( p[0] == '\r' && p[1] == '\n') {
                    *ptr = '\0';
                    p += 2;
                    break;
                }
                *ptr++ = *p++;
            }
            //printf("%s\n",line);
            err = process_line(http, line, http->linecount);
            if ( err == 0 )
                break;
            http->linecount++;
        }

    } 

    http->status |= ANT_READ_HEAD;
    /*record log*/
    write(1, response, pos ? (pos+ 4-response) : BUFSIZ);
    fprintf(stderr, "filesize = %ld: chunksize = %ld:%s\n",
            http->filesize, http->chunksize, http->down->url);
    return 0;
}


int http_read_body(struct http_info *http) {
    int fd = http->fd;
    int err;
    char line[128] = "";
    char *pos = NULL;
    char *page;
    char buf[BUFSIZ] = "";

    if (http->status & ANT_READ_BODY)
        return 0;

    err = 0;
    /*
     *Chunked transfer encoding 
     * */
    if ( http->chunksize == 0 ) {
        for (;;) {

            if (http->chunkhead == 0) {
                memset(line, 0, sizeof(line));
                if (sock_read(fd, line, sizeof(line) - 1, MSG_PEEK) < 0)
                    ant_error("http_read_body recv error");
                pos = strstr(line, "\r\n");

                if (!pos) {
                    if (!strcmp(line, "0\r"))
                        break;
                    /* debug error*/
                    fprintf(stderr, "in %s not found CRLF\n", __FUNCTION__);
                    fprintf(stderr, "%s:%s\n", line, http->down->url);
                    break;
                }

                memset(line, 0, sizeof(line));
                err = readn_block(fd, line, pos+2-line);
                if (err < 0)
                    ant_error("http_read_body:read chunked header");

                http->chunkhead = strtoul(line, NULL, 16);

                if (http->chunkhead == 0)
                    break;

            }
            page = malloc(http->chunkhead);
            if (!page)
                ant_error("malloc");

            err = readn_nonblock(fd, page, http->chunkhead, NULL);

            if (err < 0) {
                free(page);
                ant_error("http_read_body:read chunked body");
            }

            if (err > 0) {
                ant_buf_append(http->page, page, err);
                http->chunkhead -= err;
            }

            if (http->chunkhead == 0)
                readn_block(fd, line, 2);
            free(page);
            page = NULL;

            if (err == 0)
                return 0;
            fprintf(stderr, "chunkhead = %d err = %d\n",
                    http->chunkhead, err);

        }

    } else if ( http->filesize > 0 ) {
        /*
         * Content-Length header
         **/
        int len = http->filesize - http->page->ncurr;
        page = malloc(len);
        if (page == NULL)
            return -1;
        err = readn_nonblock(fd, page, len, NULL);
        /*fprintf(stderr, "err = %d\n", err);*/
        if (err > 0) {
            ant_buf_append(http->page, page, err);
            len -= err;
        }

        free(page);
        page = NULL;
        /*fprintf(stderr, "http->filesize = %ld:http->page->ncurr = %d\n", */
        /*http->filesize, http->page->ncurr);*/
        if (err < 0 || len > 0)
            return err;
    } else if (http->filesize == -1 && http->chunksize == -1) {
        /*
         * http 1.0
         * */
        for (;;) {
            err = readn_nonblock(fd, buf, BUFSIZ, &http->my_errno);
            if (err > 0)
                ant_buf_append(http->page, buf, err);
            else if (err < 0)
                return -1;
            else {
                /* err = 0 */ 
                if (http->my_errno != EAGAIN)
                    break;

                /* errno is EAGAIN */
                return -1;
            }
        }
    }

    http->status |= ANT_READ_BODY;
    fprintf(stderr, "end\n");
    return 0;
}

static int process_line( struct http_info *http, char *line, int line_count) {
    //printf("%s\n", line);
    char *p, *tag, *end;
    if (line[0] == '\0' ){
        http->end_header = 1;
        return 0;
    }

    p = line;
    if ( line_count == 0 ) {
        while( !isspace(*p) && *p ) 
            p++;

        http->http_code = strtol( p, &end, 10);
    } else {
        while( *p && *p != ':' )
            p++;

        if ( *p != ':' )
            return 1;

        *p = '\0';
        tag = line;
        p++;

        while ( isspace( *p) )
            p++;

        if (!strcasecmp(tag, "Location")) {
            strncpy(http->location, p, MAX_URL_SIZE - 1);
            http->location[MAX_URL_SIZE - 1] = '\0';
        } else if (!strcasecmp(tag, "Content-Length") && http->filesize == -1) {
            http->filesize  = strtoll(p, NULL, 10);
            http->chunksize = -1;
        } else if (!strcasecmp(tag, "Transfer-Encoding") && !strncasecmp(p, "chunked", 7)) {
            http->filesize  = -1;
            http->chunksize = 0;
        } else if (!strcasecmp(tag, "Content-Encoding")) {
            if (!strncasecmp(p, "gzip", 4))
                http->isgzip = 1;
            else {
                fprintf(stderr, "Content-Encodeing = %s\n", p);
            }
        } else if (!strcasecmp(tag, "Set-Cookie")) {
            if (!http->cookies) {
                http->cookies = strdup(p);
            } else {
                char *tmp = http->cookies;
                http->cookies = malloc(strlen(p) + strlen(tmp) + 2);
                if (http->cookies == NULL) {
                    http->cookies = tmp;
                    perror("malloc");
                    return -1;
                }
                sprintf(http->cookies, "%s\n%s", tmp, p);
                free(tmp);
            }
        } else if (!strcasecmp(tag, "Connection")) {
            if (!strcasecmp(p, "close"))
                http->close = 1;
        }
    }
    return 1;
}


char *setfilename(struct http_info *http) {

    int i, n;
    char *last = NULL;
    char *url = http->down->url;
    char *buf = http->fi.filename;

    for (i = strlen(url); i && url[i] != '/'; i--)
        ;

    /*buf = time_*/
    snprintf(buf, MAX_FILE_SIZE, "%ld_", time(NULL));

    /*if path is empty
     * buf = time_index.html
     * else buf = time_path*/
    if (url[i++] == '/') {
        strncat(buf, (url[i] == '\0') ? ".html": url+i,
                MAX_FILE_SIZE - 50);
        buf[MAX_FILE_SIZE - 50] = '\0';
    }
    else
        strcat(buf, "index.html");

    if (http->isgzip == 1)
        strcat(buf, ".gz");

    /*for example:
     *         if time_index.html exits
     * changed to time_index.html.1
     * ...
     * */

    while(access(buf, F_OK) == 0 && buf[0]) {
        for (last = buf + strlen(buf) - 1;
                buf < last && isdigit(*last); last--);
        last++;
        n = atoi(last);
        snprintf(last, MAX_FILE_SIZE - 1,
                (last[-1] == '.') ? "%d":".%d", ++n);
    }

    return buf;
}
