#include "libusc_mrcp.h"
#include "proto.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

static char *ip = "192.168.5.70";
static char *port;

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

    int s = sock_connect(ip, port);
    if (s == -1)
        return -1;
    *asr = s;

    PUSH_FUN(buf, offset);
    PUSH_STR(buf, offset, configFile);
    PUSH_INT(buf, offset, sessionNum, sizeof(unsigned int));
    PUSH_HEAD(buf, offset);

    writen(s, buf, offset + 4);

    /*read return value*/
}

int usc_mrcp_asr_destroy(HANDLE_MRCPINTF asr)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);
    PUSH_INT(buf, offset, asr, sizeof(asr)); /*asr*/
    PUSH_HEAD(buf, offset);

    /*read data*/
    return 0;
}

int usc_mrcp_service_init(HANDLE_MRCPINTF* Handle, HANDLE_MRCPINTF asr, void* obj){
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);
    PUSH_INT(buf, offset, asr, sizeof(asr)); /*asr*/
    PUSH_HEAD(buf, offset);

    /*read data*/
}

int usc_mrcp_service_start(HANDLE_MRCPINTF Handle) {
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);
    PUSH_INT(buf, offset, Handle, sizeof(Handle)); /*asr*/
    PUSH_HEAD(buf, offset);
}

int usc_mrcp_service_recognize(HANDLE_MRCPINTF Handle, char* buffer, int len, int lastSectFlag) {
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);
    PUSH_INT(buf, offset, Handle, sizeof(Handle)); /*asr*/
    PUSH_BUF(buf, offset, buffer, len);
    PUSH_INT(buf, offset, lastSectFlag, sizeof(lastSectFlag));
    PUSH_HEAD(buf, offset);
}

int usc_mrcp_service_get_recg_result(HANDLE_MRCPINTF Handle, char* recgStr, int buf_len)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);;
    PUSH_INT(buf, offset, Handle, sizeof(Handle));
    PUSH_BUF(buf, offset, recgStr, buf_len);
    PUSH_HEAD(buf, offset);
}

void usc_mrcp_service_release(HANDLE_MRCPINTF Handle)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);;
    PUSH_INT(buf, offset, Handle, sizeof(Handle));
    PUSH_HEAD(buf, offset);
}

int usc_mrcp_service_set_option(HANDLE_MRCPINTF Handle,int option_id, const char* value)
{
    char buf[48];
    off_t offset = 0;
    PUSH_FUN(buf, offset);
    PUSH_INT(buf, offset, Handle, sizeof(Handle));
    PUSH_INT(buf, offset, option_id, sizeof(option_id));
    PUSH_STR(buf, offset, value);
    PUSH_HEAD(buf, offset);
    return 0;
}

#if 1
int main(int argc, char **argv) {
    long long s;
    if (argc != 2) {
        printf("<Usage:> %s <port>\n", argv[0]);
        return -1;
    }
    port = argv[1];

    usc_mrcp_asr_init(&s, "1111111", 5);
    return 0;
}
#endif
