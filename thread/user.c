#include "proto.h"
#include "libusc_mrcp.h"
#include <pthread.h>

#define CMP(p, s) strncmp(p, s, sizeof(s) - 1)
#define SKIP_FUN(p) (4 + ntohl(*(uint32_t *)(p)))

static HANDLE_MRCPINTF user_asr = -1;

#define ASR_INIT "usc_mrcp_asr_init"
#define ASR_DESTROY "usc_mrcp_asr_destroy"
#define SERVICE_INIT "usc_mrcp_service_init"
#define SERVICE_START "usc_mrcp_service_start"
#define SERVICE_RECOGNIZE "usc_mrcp_service_recognize"
#define SERVICE_RELEASE "usc_mrcp_service_release"
#define SERVICE_SET_OPTION "usc_mrcp_service_set_option"
#define SERVICE_GET_RECG_RESULT "usc_mrcp_service_get_recg_result"

#define BEGIN(str) printf("%s:start\n", str);
//#define END(str) printf("%s:return value = %d\n", str);
static pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t user_cond = PTHREAD_COND_INITIALIZER;
static int status;

static inline void set_status_open() {
    status = 1;
}

static inline void set_status_close() {
    status = 0;
}

#define LOCK pthread_mutex_lock
#define UNLOCK pthread_mutex_unlock
#define SIGNAL pthread_cond_signal
#define WAIT   pthread_cond_wait
void process_msg(int s) {
    char *p;
    int  rv;

    int n = 0;

    p = NULL;

    uint32_t narg1;
    uint32_t offset;
    char *argbuf = NULL;

    HANDLE_MRCPINTF Handle = 0;

    for (;;) {

        /*
         * function name
         * arg1
         * arg2
         * arg3
         * ....
         * data type(function name,arg1, arg2, arg3 ...)
         * |--data len---|--data---|
         */
        rv = readvrec(s, &p, &n);
        if (rv == 0) {
            goto quit;
        }
        printf(" readvrec rv = %d\n", rv);

        offset = SKIP_FUN(p);

        printf(" offset = %d\n", offset);
        if (!CMP(p + 4, ASR_INIT)) {

            printf("%s:start user_asr = %ld\n", ASR_INIT, user_asr);
            if (user_asr != -1)
                goto loop;

            LOCK(&user_mutex);
            
            /*get function arg*/
            unsigned int arg = 0;
            POP_BUF(p, offset, n, argbuf, narg1);
            POP_INT(p, offset, n, arg, sizeof(unsigned int));

            printf("%s:configFile = %s:sessionNum = %ld\n", ASR_INIT, argbuf, arg);

            /*call function*/
            rv =usc_mrcp_asr_init(&user_asr, argbuf, narg1);

            set_status_open();
            SIGNAL(&user_cond);
            UNLOCK(&user_mutex);

            printf("%s:user_asr = %ld\n", ASR_INIT, user_asr);

            printf("%s:return value = %d\n", ASR_INIT, rv);
        } else { 
            printf(" status = %d\n", status);
            if (!CMP(p + 4, ASR_DESTROY)) {
                BEGIN(ASR_DESTROY);

                rv =usc_mrcp_asr_destroy(user_asr);

                printf("%s:return value = %d\n", ASR_DESTROY, rv);

                /*clean up*/
                user_asr = -1;
                send_rv(s, rv);
                set_status_close();
                goto quit;
            } else if (!CMP(p + 4, SERVICE_INIT)) {
                LOCK(&user_mutex);
                while (status == 0) {
                    WAIT(&user_cond, &user_mutex);
                }
                UNLOCK(&user_mutex);
                BEGIN(SERVICE_INIT);

                rv =usc_mrcp_service_init(&Handle, user_asr, NULL);

                printf("%s:return value = %d\n", SERVICE_INIT, rv);
            } else if (!CMP(p + 4, SERVICE_START)) {
                BEGIN( SERVICE_START);

                rv =usc_mrcp_service_start(Handle);

                printf("%s:return value = %d\n", SERVICE_START, rv);
            } else if (!CMP(p + 4, SERVICE_RECOGNIZE)) {
                BEGIN( SERVICE_RECOGNIZE);

                int arg1 = 0;
                POP_BUF(p, offset, n, argbuf, narg1);
                POP_INT(p, offset, n, arg1, sizeof(int));

                printf("%s:len = %d: lastSectFlag = %d\n", SERVICE_RECOGNIZE, narg1, arg1);
                rv =usc_mrcp_service_recognize(Handle, argbuf, narg1, arg1);

                printf("%s:return value = %d\n", SERVICE_RECOGNIZE, rv);
            } else if (!CMP(p + 4, SERVICE_GET_RECG_RESULT)) {
                printf("%s:\n", SERVICE_GET_RECG_RESULT);

                /* head 4 byte 
                 * '\0' 1 byte
                 */
                rv =usc_mrcp_service_get_recg_result(Handle, p + 4, n - 5);
                proto_add_head(p , rv);
                writen(s, p, rv + 4);

                printf("%s:return value = %d:%s\n", SERVICE_GET_RECG_RESULT, rv, p + 4);
            } else if (!CMP(p + 4, SERVICE_RELEASE)) {
                BEGIN( SERVICE_RELEASE);

                usc_mrcp_service_release(Handle);

                printf("%s:end\n", SERVICE_RELEASE);
            } else if (!CMP(p + 4, SERVICE_SET_OPTION)) {
                BEGIN( SERVICE_SET_OPTION);

                int arg = 0;
                POP_INT(p, offset, n, arg, sizeof(int));
                POP_BUF(p, offset, n, argbuf, narg1);
                printf("%s:option_id = %d:value = %s\n", SERVICE_SET_OPTION, arg, argbuf);

                rv =usc_mrcp_service_set_option(Handle, arg, argbuf);

                printf("%s:retrun value = %d\n", SERVICE_SET_OPTION, rv);
            }
        }
loop:
        free(argbuf);
        argbuf = NULL;
    }

quit:
    free(p);
    free(argbuf);
    close(s);
    
    printf("Bye Bye sock fd = %d\n\n============================\n", s);
}
