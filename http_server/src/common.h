#ifndef _COMMON_H
#define _COMMON_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <strings.h>
# include <ctype.h>
# include <time.h>
# include <assert.h>
# include <errno.h>

# include <unistd.h>
# include <dirent.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <stdarg.h>

# include "tor_http.h"
# include "tor_socket.h"
# include "tor_log.h"
# include "tor_string.h"
# include "tor_array.h"

#define TOR_PATH 256

typedef struct tor_config {
    char data_dir[TOR_PATH];
    char logfile[TOR_PATH];
    unsigned int   ip;
    unsigned short port;
} config_t;

config_t config;



#define IS_DIR(fname)         ({      \
        struct stat tor__sb;          \
        tor__sb.st_mode = 0;          \
        stat(fname, &tor__sb);        \
        S_ISDIR(tor__sb.st_mode);     \
        })

#endif
