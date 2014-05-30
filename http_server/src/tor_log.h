#ifndef _TOR_LOG_H
#define _TOR_LOG_H

extern void tor_log_err(char *fmt, ...);

extern void tor_log_clean(void);

extern void tor_log_core(char *str, size_t n, char *fmt, va_list ap);
#endif
