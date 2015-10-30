#include <stdio.h>
#include "str_match.h"

struct tst_str_opt{
    char *str;
    char *fmt;
    char *msg;
};

int main() {

    struct tst_str_opt arr[] = {
        /* test %d */
        {"12342", "%d", "match ok"}, /* match ok */
        {"abcd", "%d", "match no"},  /* match no*/
        {"192.168.1.1", "%d.%d.%d.%d", "match ok"}, /* match ok*/

        /* test %width d */
        {"1234567891", "%10d", "match ok"}, /* match ok */
        {"12345", "%2d", "match no"}, /* match no */

        /* test %[] */
        {"aa:bb:cc:dd:ee:ff", "%[a-z]:%[a-z]:%[a-z]:%[a-z]:%[a-z]:%[a-z]", "match ok"}, /* match ok*/
        {"Aa:Bb:Cc:Dd:Ee:Ff", "%[a-zA-Z]:%[a-zA-Z]:%[a-zA-Z]:%[a-zA-Z]:%[a-zA-Z]:%[a-zA-Z]", "match ok"}, /* match ok*/
        {"Aa:Bb:Cc:Dd:Ee:Ff", "%[a-z]:%[a-z]:%[a-z]:%[a-z]:%[a-z]:%[a-z]", "match no"}, /* match no*/
        {"aa:bb:cc:dd:ee:ff", "%2[a-z]:%2[a-z]:%2[a-z]:%2[a-z]:%2[a-z]:%2[a-z]", "match ok"}, /* match ok*/
        {"aa:bbb:cc:dd:ee:ff", "%2[a-z]:%2[a-z]:%2[a-z]:%2[a-z]:%2[a-z]:%2[a-z]", "match no"}, /* match no*/
        {" \t\r\n", "%[ \t\r\n]", "match ok"}, /*match ok*/

        /* test %f */
        {"1.1", "%f", "match ok"}, /*match ok*/
        {"111", "%f", "match ok"}, /*match ok*/
        {"111", "%2f" "match no"}, /*match no*/
        {"12", "%2f", "match ok"}, /*match ok*/
        {"192.168.1.1", "%f", "match no"}, /*match no*/

        /* test [^...] */
        {"aa", "%[^0-9]", "match ok"}, /*match ok*/
        {"AZ", "%[^0-9]", "match ok"}, /*match ok*/
        {"  \t\r\n", "%[^0-9]", "match ok"}, /*match ok*/
        {"aa", "%[^a-z]", "match no"}, /*match no*/
        {"aa", "%[^A-Z]", "match ok"}, /*match ok*/
        {"1234", "%[^0-9]", "match no"}, /*match no*/
        {"abcd", "%4[^0-9]", "match ok"}, /*match ok*/
        {"abcd", "%2[^0-9]", "match no"}, /*match no*/


        /* test %? */
        {"https://", "https%?://", "match ok"}, /*match ok*/
        {"http://", "https%?://", "match ok"},   /*match ok*/
        {"h", "h%?", "match ok"},              /*match ok*/
        {"h", "h%?", "match ok"},              /*match ok*/
        {"", "w%?w%?", "match ok"},              /*match ok*/

        /* test {|}*/
        {".css", "%{.css|.js|.png}", "match ok"}, /*match ok*/
        {".png", "%{.css|.js|.png}", "match ok"}, /*match ok*/
        {".js", "%{.css|.js|.png}", "match ok"}, /*match ok*/
        {".xml", "%{.css|.js|.png}", "match no"}, /*match no*/
        {".png|", "%{.css|.js|.png}", "match no"}, /*match no*/
        {".png|", "%{.css|.js|.png}|", "match ok"}, /*match ok*/
        {".png.3", "%{.js|.png}.%{one|two}", "match no"}, /*match no*/
        {".png.one", "%{.js|.png}.%{one|two}", "match ok"}, /*match ok*/

#if 0
        /* test %* */
        {"1.txt", "%*.txt"}, /*match ok*/
        {"2.txt", "%*.txt"}, /*match ok*/
        {"3.txt", "%*.txt"}, /*match ok*/
#endif

    };

    int i, len;
    len = sizeof(arr)/sizeof(arr[0]);
    for (i = 0; i < len; i++) {
        printf("str = %s: fmt = %s: %s: %s\n", 
                arr[i].str, arr[i].fmt, arr[i].msg,
                str_match(arr[i].str, arr[i].fmt) ? "match ok": "match no");
        printf("\n");
    }
    return 0;
}
