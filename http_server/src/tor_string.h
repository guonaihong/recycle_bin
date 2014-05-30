#ifndef _TOR_STRING_H
#define _TOR_STRING_H

/*
 * how to use sprintfcat
 *    char *p  = NULL;
 *    int use  = 0;
 *    int size = 0;
 *    ...
 *    sprintfcat(&p, &use, &size, "aaa");
 *    ...
 *    sprintfcat(&p, &use, &size, "%d", 3);
 *    ....
 */
int sprintfcat(char **strp, int *use, int *size, char *fmt, ...);
#endif
