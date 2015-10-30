#ifndef ANT_STRING_H
#define ANT_STRING_H


#define  ISSPACE_LINE(p) ({                                     \
     char *_ant_line_ = p;                                      \
     for (; *_ant_line_ && isspace(*_ant_line_); _ant_line_++); \
     *_ant_line_ == '\0'; })

char *ant_dump_num(char *dst, unsigned int ui, int base);

int ant_iszero(void *s, size_t n);

double ant_atod(char *s, char **endptr);

char *ant_itoa(int n, char *dst, int base);

int add_buf(char **buf, int *cur_len, int *bufsize, char *str, int len);
#endif
