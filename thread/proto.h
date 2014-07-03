#ifndef _PROTO_H
#define _PROTO_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
/*protocol
 * function name
 * argv of number
 * first argv
 * section argv
 * ....
 */
static inline int proto_encode(char *outbuf, off_t *offset, uint32_t outlen, const char *inbuf, uint32_t inlen) {
	if (outlen < inlen)
		return -1;

	if (offset) {
		if (outlen < *offset + inlen)
			return -1;

		*(uint32_t *)(outbuf + *offset) = htonl(inlen);/*set data header*/
		memcpy(outbuf + *offset + 4, inbuf, inlen); /*set data*/

		*offset += inlen + 4;
	} else {
		if (inlen != 4)
			return -1;
		*(uint32_t *)outbuf = htonl(*(uint32_t *)inbuf);
	}
    return 0;
}

static inline int proto_decode(const char *inbuf, off_t *offset, size_t inlen, char *outbuf, size_t outlen) {
    uint32_t len;
    if (*offset >= inlen)
        return -1;

    len = *(uint32_t *)(inbuf + *offset);
    len = ntohl(len);
    if (len > outlen)
        return -1;

    *offset += 4;/*skip sizeof(uint32_t)*/
    memcpy(outbuf, inbuf + *offset, len);
    offset += len;

    return 0;
}
/*protocol*/

static int endian = 1;
/**/
#define swap64(val) (((val) >> 56) |\
		(((val) & 0x00ff000000000000ll) >> 40) |\
		(((val) & 0x0000ff0000000000ll) >> 24) |\
		(((val) & 0x000000ff00000000ll) >> 8)   |\
		(((val) & 0x00000000ff000000ll) << 8)   |\
		(((val) & 0x0000000000ff0000ll) << 24) |\
		(((val) & 0x000000000000ff00ll) << 40) |\
		(((val) << 56)))

#define hton64(x) ((*(char *)&endian ) ? swap64(x) :(x))
#define ntoh64(x) ((*(char *)&endian ) ? swap64(x) :(x))

inline static int proto_encode_int(char *outbuf, off_t *offset, uint32_t outlen, const char *inbuf, size_t inlen) {
	if (inlen == 2) {
		uint16_t u16 = htons(*(uint16_t *)inbuf);
		return proto_encode(outbuf, offset, outlen, (char *)&u16, 2);
	}
	else if (inlen == 4) {
		uint32_t u32 = htonl(*(uint32_t *)inbuf);
		return proto_encode(outbuf, offset, outlen, (char *)&u32, 4);
	}
	else if (inlen == 8) {
		uint32_t u64 = hton64(*(uint64_t *)inbuf);
		return proto_encode(outbuf, offset, outlen, (char *)&u64, 8);
	}

	return proto_encode(outbuf, offset, outlen, inbuf, inlen);
}

#define PUSH_FUN(buf, offset) \
	proto_encode(buf + 4, &offset, sizeof(buf), __func__, strlen(__func__))

#define PUSH_STR(buf, offset, str) \
	proto_encode(buf, &offset, sizeof(buf), str, strlen(str));

#define PUSH_HEAD(buf, offset)  \
	proto_encode(buf, NULL, sizeof(buf), (char *)&offset, 4);

#define PUSH_BUF(buf, offset, inbuf, inlen) \
	proto_encode(buf, &offset, sizeof(buf), inbuf, inlen);

#define PUSH_INT(buf, offset, data, len)  \
	proto_encode_int(buf, &offset, sizeof(buf), (char *)&data, len);

inline static ssize_t writen(int s, const void *buf, size_t count) {
    char    *p  = (char *)buf;
    ssize_t len = count;
    ssize_t rv  = 0;

    while (len > 0) {
		rv = write(s, p, len);

        if (rv == -1) {
            if (errno == EINTR)
                continue;
            break;
        }

        if (rv == 0)
            return count - len;

        p   += rv;
        len -= rv;
    }

    return count - len;
}

inline static ssize_t readn(int s, const void *buf, size_t count) {
    char     *p  = (char *)buf;
    ssize_t  len = count;
    ssize_t  rv  = 0;

    while (len > 0) {
        rv = read(s, p, len);

        if (rv == -1) {
            if (errno == EINTR)
                continue;
            break;
        }

        if (rv == 0)
            return count - len;

        p   += rv;
        len -= rv;
    }

    return count - len;
}
//#undef swap64
#endif
