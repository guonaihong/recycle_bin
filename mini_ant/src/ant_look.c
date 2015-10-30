#include "ant_look.h"
#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

char text_chars[256] = {
    /*                  BEL BS HT LF    FF CR    */
    F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
    /*                              ESC          */
    F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
    /*            NEL                            */
    X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
};


/*file_looks_utf8 function from gnu file command source
 * URL : http://www.darwinsys.com/file/ */
/*
 *Decide whether some text looks like UTF-8. Returns:
 *
 * -1: invalid UTF-8
 * 0: uses odd control characters, so doesn't look like text
 * 1: 7-bit text
 * 2: definitely UTF-8 text (valid high-bit set bytes)
 *
 * If ubuf is non-NULL on entry, text is decoded into ubuf, *ulen;
 * ubuf must be big enough!
 */

int file_looks_utf8(const unsigned char *buf, size_t nbytes) {
    size_t i;
    int n;
    int gotone = 0, ctrl = 0;

    for (i = 0; i < nbytes; i++) {
        if ((buf[i] & 0x80) == 0) {	   /* 0xxxxxxx is plain ASCII */
            /*
             * Even if the whole file is valid UTF-8 sequences,
             * still reject it if it uses weird control characters.
             */

            if (text_chars[buf[i]] != T)
                ctrl = 1;

        } else if ((buf[i] & 0x40) == 0) { /* 10xxxxxx never 1st byte */
            return -1;
        } else {			   /* 11xxxxxx begins UTF-8 */
            int following;

            if ((buf[i] & 0x20) == 0) {		/* 110xxxxx */
                following = 1;
            } else if ((buf[i] & 0x10) == 0) {	/* 1110xxxx */
                following = 2;
            } else if ((buf[i] & 0x08) == 0) {	/* 11110xxx */
                following = 3;
            } else if ((buf[i] & 0x04) == 0) {	/* 111110xx */
                following = 4;
            } else if ((buf[i] & 0x02) == 0) {	/* 1111110x */
                following = 5;
            } else
                return -1;

            for (n = 0; n < following; n++) {
                i++;
                if (i >= nbytes)
                    goto done;

                if ((buf[i] & 0x80) == 0 || (buf[i] & 0x40))
                    return -1;
            }

            gotone = 1;
        }
    }
done:
    return ctrl ? 0 : (gotone ? 2 : 1);
}
