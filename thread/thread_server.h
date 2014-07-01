#ifndef THREAD_SERVER_H
#define THREAD_SERVER_H
/*protocol
 * function name length:function name
 * argv of number
 * first argv
 * section argv
 * ....
 */
int proto_code(char *outbuf, off_t *offset, uint32_t outlen, const char *inbuf, uint32_t inlen);

int proto_decode(const char *inbuf, off_t *offset, size_t inlen, char *outbuf, size_t outlen);
/*protocol*/
#endif
