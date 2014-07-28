#include "libusc_mrcp.h"
#include "proto.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static char *ip = "192.168.5.70";
static char *port = "1234";

#if 0
static void private_init() {
    if (!ip)
        ip = getenv("RPC_IP");

    if (!ip) {
        fprintf(stdout, "please set RPC_IP=val \n");
        exit(1);
    }

    if (!port)
        port = getenv("RPC_PORT");

    if (!port) {
        fprintf(stdout, "please set RPC_PORT=val \n");
        exit(1);
    }
    fprintf(stdout, "id = %ld ip = %s:port = %s\n",pthread_self(), ip, port);
}
#endif

static int sock_connect(char *ip, char *port) {
    struct sockaddr_in client;
    int s;
    int r;
    s = socket(AF_INET, SOCK_STREAM, 0); 
    if (s == -1) {
        perror("socket");
        return -1;
    }

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    if (inet_aton(ip, &client.sin_addr) == 0) {
        perror("inet_aton");
        return -1;
    }   
    client.sin_port = htons(atoi(port));
    r = connect(s, (struct sockaddr*)&client, sizeof(client));

    if (r != 0) {
        perror("connect");
        return -1;
    }   

    return s;
}

/*
 *|---------total len------------|
 *|--4 bytes---|offset len-------|
 */
int usc_mrcp_asr_init(HANDLE_MRCPINTF* asr, const char* configFile,unsigned int sessionNum)
{
    char buf[512];
    off_t offset =  0;

    //private_init();
    int s = sock_connect(ip, port);
    if (s == -1)
        return -1;
    *asr = s;

    PUSH_FUN(buf, offset, sizeof(buf));
    PUSH_STR(buf, offset, sizeof(buf), configFile);
    PUSH_INT(buf, offset, sizeof(buf), sessionNum, sizeof(unsigned int));
    PUSH_HEAD(buf, offset, sizeof(buf));

    writen(s, buf, offset + 4);

    /*read return value*/
    return 0;
}

int usc_mrcp_asr_destroy(HANDLE_MRCPINTF asr)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset, sizeof(buf));
    PUSH_HEAD(buf, offset, sizeof(buf));

    writen(asr, buf, offset + 4);
    /*read return value*/
    readn(asr, buf, 8);
    close(asr);
    return 0;
}

int usc_mrcp_service_init(HANDLE_MRCPINTF* Handle, HANDLE_MRCPINTF asr, void* obj){
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset, sizeof(buf));
    PUSH_HEAD(buf, offset, sizeof(buf));


    int s = sock_connect(ip, port);
    if (s == -1)
        return -1;
    *Handle = s;

    writen(*Handle, buf, offset + 4);
    return 0;
    /*read data*/
}

int usc_mrcp_service_start(HANDLE_MRCPINTF Handle) {
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset, sizeof(buf));
    PUSH_HEAD(buf, offset, sizeof(buf));

    writen(Handle, buf, offset + 4);

    return 0;
}

int usc_mrcp_service_recognize(HANDLE_MRCPINTF Handle, char* buffer, int len, int lastSectFlag) {
    int size;
    off_t offset = 0;
    char *buf= NULL;

    size = 4 + sizeof(__func__) - 1 +
           4 + len +
           4 + sizeof(lastSectFlag);

    buf= malloc(size);
    if (!buf) {
        fprintf(stderr, "%s:%d malloc %m\n", __FILE__, __LINE__);
        return -1;
    }
    PUSH_FUN(buf, offset, size);
    PUSH_BUF(buf, offset, size, buffer, len);
    PUSH_INT(buf, offset, size, lastSectFlag, sizeof(lastSectFlag));
    PUSH_HEAD(buf, offset, size);

    writen(Handle, buf, offset + 4);

    free(buf);
    return 0;
}

int usc_mrcp_service_get_recg_result(HANDLE_MRCPINTF Handle, char* recgStr, int buf_len)
{
    char buf[48];
    off_t offset = 0;
    int rv;
    PUSH_FUN(buf, offset, sizeof(buf));
    PUSH_HEAD(buf, offset, sizeof(buf));

    writen(Handle, buf, offset + 4);

    rv = readvrec1(Handle, recgStr, buf_len - 1);
    printf("rv = %d\n", rv);
    if (rv > 0)
        recgStr[rv] = '\0';
    /*TODO read sock*/
    return rv;
}

void usc_mrcp_service_release(HANDLE_MRCPINTF Handle)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset, sizeof(buf));;
    PUSH_HEAD(buf, offset, sizeof(buf));

    writen(Handle, buf, offset + 4);

    close(Handle);
}

int usc_mrcp_service_set_option(HANDLE_MRCPINTF Handle,int option_id, const char* value)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset, sizeof(buf));
    PUSH_INT(buf, offset, sizeof(buf), option_id, sizeof(option_id));
    PUSH_STR(buf, offset, sizeof(buf), value);
    PUSH_HEAD(buf, offset, sizeof(buf));

    writen(Handle, buf, offset + 4);
    return 0;
}

static int file2mem(char *fname, char **p, int *size) {
    char *mem = NULL;
    struct stat sb = {0}; 
    int fd = -1; 

    if(stat(fname, &sb) == -1) {
        perror("stat");
        return -1; 
    }   

    mem = malloc(sb.st_size);
    if (!mem) {
        perror("MALLOC");
        return -1; 
    }   

    fd = open(fname, O_RDONLY, 0644);
    if (fd == -1) {
        free(mem);
        perror("open");
        return -1; 
    }   

    read(fd, mem, sb.st_size);
    *p = mem;
    *size = sb.st_size;
    close(fd);
    return 0;
}
#if GUO_TEST
#define MAX_RECOG_TEXT_LEN 1024
int main(int argc, char **argv) {

    int r;
    char *p = NULL;
    int n = 0;
    int size;

	int lastSectFlag = 1; 

	HANDLE_MRCPINTF asr_handle;
	HANDLE_MRCPINTF service_handle;
	char text[MAX_RECOG_TEXT_LEN] = "";
	r = usc_mrcp_asr_init(&asr_handle, "asrConfig/ctx.ini", 1);
	if(r < 0){
		printf("\n usc_mrcp_asr_init failed\n");
	}
    printf("\n usc_mrcp_asr_init OK!\n");

	r = usc_mrcp_service_init( &service_handle, asr_handle, NULL);
	if(r < 0){
		printf("\n usc_mrcp_service_init failed\n");
	}
    printf("\r\nusc_mrcp_service_init OK!\n");

	usc_mrcp_service_set_option(service_handle, USC_SAMPLE_RATE, "8k");
	r = usc_mrcp_service_start(service_handle);
	if(r < 0){
		printf("\n usc_mrcp_service_start failed\n");
	}
    printf("\n usc_mrcp_service_start OK!\n");

    file2mem("test.wav", &p, &n);
	usc_mrcp_service_set_option(service_handle, USC_SAMPLE_RATE, "8k");
    printf("usc_mrcp_service_set_option ok\n");

	usc_mrcp_service_recognize( service_handle, p + 44, n - 44, lastSectFlag);
    printf("usc_mrcp_service_recognize ok\n");

	size = usc_mrcp_service_get_recg_result(service_handle , text, MAX_RECOG_TEXT_LEN-1);
    printf("usc_mrcp_service_get_recg_result ok\n");
	printf(":::::: asr result: %s\n",text);

	usc_mrcp_service_release(service_handle);
    printf("usc_mrcp_service_release OK!\n");

    usc_mrcp_asr_destroy(asr_handle);
    printf("usc_mrcp_asr_destroy OK!\n");
    return 0;
}
#endif
