#ifndef _ANT_LOG_H
#define _ANT_HLOG_H

extern void ant_log_url_all(const char *fmt, ...);

extern void ant_log_err(const char *fmt, ...);

extern void ant_log_url(const char *fmt, ...);

extern void ant_log_init(void);

extern void ant_log_clean(void);

extern void ant_log_core(char *str, size_t n, const char *fmt, va_list ap);

#define ANT_LOG_RECORD_NAME "log/down.log"
#define ANT_LOG_RECORD_ERR_NAME "log/err.log"
#define ANT_LOG_RECORD_URL_NAME "log/url.log"
#endif
