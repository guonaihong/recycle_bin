#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ant_log.h"

static int s_url;
static int s_url_all;
static int s_err;

static char url_line[128];
static char err_line[512];
static char url_all_line[4096];

/* down url */
static void ant_log_url_init(void) {
    s_url = open(ANT_LOG_RECORD_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (s_url < 0) {
        fprintf(stderr, "%s:%m\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
}

/*url all */
static void ant_log_url_all_init(void) {
    s_url_all = open(ANT_LOG_RECORD_URL_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (s_url_all < 0) {
        fprintf(stderr, "%s:%m\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
}

/* error */
static void ant_log_err_init(void) {
    s_err = open(ANT_LOG_RECORD_ERR_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (s_err < 0) {
        fprintf(stderr, "%s:%m\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
}

static void ant_log_err_close(void) {
    close(s_err);
    s_err = -1;
}

static void ant_log_url_all_close(void) {
    close(s_url_all);
    s_url_all = -1;
}

static void ant_log_url_close(void) {
    close(s_url);
    s_url = -1;
}

void ant_log_init() {
    ant_log_url_init();
    ant_log_url_all_init();
    ant_log_err_init();
}

void ant_log_clean() {
    ant_log_err_close();
    ant_log_url_all_close();
    ant_log_url_close();
}

/* download url write file */
void ant_log_url(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(url_line, sizeof(url_line), fmt, ap);
    va_end(ap);

    write(s_url, url_line, strlen(url_line));
}

/* error write file */
void ant_log_err(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ant_log_core(err_line, sizeof(err_line), fmt, ap);
    va_end(ap);

    write(s_err, err_line, strlen(err_line));
}

/* all written document parsing url */
void ant_log_url_all(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ant_log_core(url_all_line, sizeof(url_all_line), fmt, ap);
    va_end(ap);

    write(s_url_all, url_all_line, strlen(url_all_line));
}

void ant_log_core(char *str, size_t n, const char *fmt, va_list ap) {
    time_t t;
    struct tm tmval;
    int len;

    t = time(NULL);
    tmval = *localtime(&t);
    snprintf(str, n,"%d-%02d-%02d %02d:%02d:%02d ",
            1900 + tmval.tm_year, tmval.tm_mon + 1, tmval.tm_mday,
            tmval.tm_hour, tmval.tm_min, tmval.tm_sec);

    len = strlen(str);

    vsnprintf(str + len, n - len, fmt, ap);
}

#if _ANT_TEST
int main() {

    ant_log_init();
    ant_log_url_all("hello world = %d\n", 222);
    ant_log_url("%s:::%d\n", __FUNCTION__, 111111);
    ant_log_err("%s:::%d\n", __FUNCTION__, 111111);
    ant_log_clean();
    return 0;
}
#endif
