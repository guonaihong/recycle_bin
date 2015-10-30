#ifndef _ANT_DNS_H
#define _ANT_DNS_H
#include <netinet/in.h>
/* from RFC 1035*/
struct dns_header{
    /*   ID 
     *   A 16-bit identifier assigned by the entity that generates any
     *   kind of request.  This identifier is copied in the
     *   corresponding reply and can be used by the requestor to match
     *   replies to outstanding requests, or by the server to detect
     *   duplicated requests from some requestor.*/
    uint16_t id;

    uint16_t flags;

    uint16_t questions;

    uint16_t answers_rrs;

    uint16_t authority_rrs;  

    uint16_t additional_rrs;  

};

struct dns_question {
    /*
     * QNAME
     * a domain name represented as a sequence of labels, where
     * each label consists of a length octet followed by that
     * number of octets.  The domain name terminates with the
     * zero length octet for the null label of the root.  Note
     * that this field may be an odd number of octets; no
     * padding is used.
     * */
    uint8_t *qname;

    /*QTYPE
     * a two octet code which specifies the type of the query.
     * The values for this field include all codes valid for a
     * TYPE field, together with some more general codes which
     * can match more than one type of RR.*/
    uint16_t qtype;

    /*QCLASS
     * a two octet code that specifies the class of the query.
     * For example, the QCLASS field is IN for the Internet.
     * */
    uint16_t qclass;
};

struct dns_answer{
    /* a domain name to which this resource record pertains.*/
    unsigned char * name;

    /* two octets containing one of the RR type codes.  This
     * field specifies the meaning of the data in the RDATA
     * field.*/
    uint16_t type;

    /* two octets which specify the class of the data in the
     * RDATA field.*/
    uint16_t class;

    /*a 32 bit unsigned integer that specifies the time
     * interval (in seconds) that the resource record may be
     * cached before it should be discarded.  Zero values are
     * interpreted to mean that the RR can only be used for the
     * transaction in progress, and should not be cached.*/
    int32_t ttl;

    /*an unsigned 16 bit integer that specifies the length in
     *octets of the RDATA field.*/
    uint16_t rdlength;

    /* a variable length string of octets that describes the
     *resource.  The format of this information varies
     *according to the TYPE and CLASS of the resource record.
     *For example, the if the TYPE is A and the CLASS is IN,
     *the RDATA field is a 4 octet ARPA Internet address.*/
    unsigned char* rdata;
};

#define IP_LEN 4
typedef struct dns_respone_s{
    char host_name[1024];
    unsigned ip[IP_LEN];
    int flags;
}dns_response_t;

int tcp_create_sock(struct sockaddr_in *addr, char *ip, short port);

void *thread_dns_main(void *argv);
#endif

