#include "common.h"
static int tor_err;
static char err_line[1024];

void tor_log_init() {
    tor_err = open(config.logfile, O_RDWR | O_CREAT | O_APPEND, 0644);

#ifdef _TOR_TEST
    fprintf(stderr, ":config.logfile = %s:\n", config.logfile);
#endif
    if (tor_err < 0) {
        fprintf(stderr, "open '%s' error :%m:%s:%d\n", 
                config.logfile, __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

void tor_log_err(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    tor_log_core(err_line, sizeof(err_line), fmt, ap);
    va_end(ap);

    write(tor_err, err_line, strlen(err_line));
}

void tor_log_core(char *str, size_t n, char *fmt, va_list ap) {
    time_t t;
    struct tm tmval;
    int len;

    time(&t);
    localtime_r(&t, &tmval);

    snprintf(str, n,"%d-%02d-%02d %02d:%02d:%02d ",
            1900 + tmval.tm_year, tmval.tm_mon + 1, tmval.tm_mday,
            tmval.tm_hour, tmval.tm_min, tmval.tm_sec);

    len = strlen(str);
    vsnprintf(str + len, n - len, fmt, ap);
}

void tor_log_clean(void) {
    close(tor_err);
}
