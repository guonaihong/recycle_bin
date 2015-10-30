#ifndef _HO_TIME_H
#define _HO_TIME_H

#define diff_msec(btv, etv, ot, sentence)  \
    gettimeofday(btv, NULL);  \
    sentence;          \
    gettimeofday(etv, NULL);  \
    *(ot) = (((etv)->tv_sec - (btv)->tv_sec) * 1000 + ((etv)->tv_usec - (btv)->tv_usec) / 1000);

#endif
