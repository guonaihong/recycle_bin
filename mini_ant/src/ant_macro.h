/*
 *Copyright (C) guonaihong
 */
#ifndef _ANT_HEAD_H
#define _ANT_HEAD_H

#define  ISSPACE_LINE(p) ({                   \
        char *_ant_line_ = p;                          \
        for (; *_ant_line_ && isspace(*_ant_line_); _ant_line_++); \
        *_ant_line_ == '\0'; })

#define DIR_EXISTS(dname) ({                 \
        struct stat _ant_statbuf_;           \
        _ant_statbuf_.st_mode = 0;           \
        lstat(dname, &_ant_statbuf_);        \
        S_ISDIR(_ant_statbuf_.st_mode); })


#define ant_error(msg ) \
    do{ perror(msg); return -1;}while(0)

#define ARRAY_LEN(arr) \
    (sizeof(arr) / sizeof((arr)[0]))

#define PRINT_IP(ip) \
    fprintf(stderr, "%u.%u.%u.%u\n",  \
            (ip) & 0xff , ((ip) >> 8) & 0xff, ((ip) >> 16) & 0xff, ((ip) >> 24) & 0xff)
#endif
