#include <stdio.h>

void reverse_words(char *s) {
    char *b = s;
    char *e = NULL;

    e = b + 1;
    while (e)
}

int main() {
#if 0
    向右旋转
    "the sky is blue";
    //第一趟
    "e the sky is blu";
    "ue the sky is bl";
    "lue the sky is b";
    "blue the sky is";
    //第二趟
    "blue s the sky i";
    "blue is the sky ";
    //第三趟
    "blue is y the sk";
    "blue is ky the s";
    "blue is sky the";

    "blue is sky the";
#endif

#if 0
    向左旋转
    "the sky is blue";
    //第一趟
    "the sky is blue ";
    "he sky is blue t";
    "e sky is blue th";
    " sky is blue the";
    "sky is blue the";
    //第二趟
    "ky is blue s the";
    "y is blue sk the";
    "is blue sky the";
    //第三趟
    "s blue i sky the";
    "blue is sky the";

    "blue is sky the";
#endif
    return 0;
}
