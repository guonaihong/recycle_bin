#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ho_read_conf.h"

static int ho_read_conf_core(const char *fname, struct ho_rc_t *rc);
static void ho_read_conf_check(struct ho_rc_t *rc);

int ho_read_conf(const char *fname, struct ho_rc_t *rc) {
    int rv;
    rv = ho_read_conf_core(fname, rc);
    if (rv == -1)
        return rv;
    ho_read_conf_check(rc);
    return 0;
}

static void ho_read_conf_check(struct ho_rc_t *rc) {
    struct ho_rc_t *rcp;

    for (rcp = rc; 
            rcp->key || rcp->out_val || 
            rcp->cb_get_val || rcp->not_empty
            ; rcp++) {
        if (rcp->not_empty == 0) {
            printf("get value of %s failed\n", rcp->key);
        }
    }
}

static int ho_read_conf_core(const char *fname, struct ho_rc_t *rc) {
    FILE *fp;
    char *key, *val, *p;
    char line[1024];
    struct ho_rc_t *rcp;

    fp = fopen(fname, "r");
    if (!fp)
        return -1;

    while (fgets(line, sizeof(line), fp)) {
loop:
        if ((p = strchr(line, '#'))) {
            *p = '\0';
        }

        p = line;
        key = p += strspn(p, " \t"); /*key begin*/

        if (*p == '\r' || *p == '\n')/*empty row*/
            continue;

        p += strcspn(p, " \t=:");
        if (!*p)
            continue;

        *p++ = '\0';/*key end*/

        val = p += strspn(p, " \t=:"); /*value begin*/
        p += strcspn(p, "\r\n");/*value end*/
        *p = '\0';

        if (*val == '\0')
            continue;

        for (rcp = rc; rcp->key || rcp->out_val || 
                rcp->cb_get_val || rcp->not_empty
                ; rcp++) {

            if (!strcmp(rcp->key, key)) {
                rcp->cb_get_val(rcp, val);
                rcp->not_empty = 1;
                goto loop;
            }
        }
        printf("%s:unrecognized option `%s'\n", __func__, key);
        //printf("key=%s:val=%s:\n", key, val);
    }

    fclose(fp);
    return 0;
}

#if 0
int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage: <%s> filename\n", argv[0]);
        return 1;
    }

    struct tst_obj {
        int            age;
        unsigned       ip;
        unsigned short port;
        double         score;
        char           *text;
#ifdef __cplusplus
        std::string         name;
#endif
    };

    struct tst_obj tst_conf, *conf;

    conf = &tst_conf;

    struct ho_rc_t rc_arr[] = {
        {ho_get_int, "age", &conf->age, 0, 0},
        {ho_get_ip,  "ip", &conf->ip, 0, 0},
        {ho_get_port,"port", &conf->port, 0, 0},
        {ho_get_double, "score", &conf->score, 0, 0},
        {ho_get_str_alloc, "text", &conf->text, 0, 0},
#ifdef __cplusplus
        {ho_get_str_cxx,"name", &conf->name, 0, 0},
#endif
        {0, 0, 0, 0, 0}
    };

    ho_read_conf(argv[1], rc_arr);

    printf("age = %d \nscore = %lf \ntext = %s\nport = %d\n",
            conf->age, conf->score, conf->text, conf->port);

    unsigned ip = conf->ip;
    printf("ip = %hhu.%hhu.%hhu.%hhu\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);

#ifdef __cplusplus
    printf("name = %s\n", conf->name.c_str());
#endif
    return 0;
}
#endif
