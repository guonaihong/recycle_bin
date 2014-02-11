#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *搜索的输入信息是一个字符串，
 *统计300万输入信息中的最热门的前10条，
 *我们每次输入的一个字符串为不超过255byte，
 *内存使用只有1G。
 *请描述思想，写出算法（c语言），空间和时间复杂度。
 */

struct hash_url {
    char *url;
    int count;
    struct hash_url *next;
};

#define SIZE 1024
struct hash_head {
    struct hash_url *tab[SIZE];
};

unsigned hash_num(char *s, int size) {
    unsigned hashval = 0;

    while (*s) {
        hashval = *s++ + 31 * hashval;
    }

    return hashval % size;
}

static struct hash_url *lookup(struct hash_head *head, char *url) {
    unsigned hash = hash_num(url, SIZE);
    struct hash_url *p = head->tab[hash];

    for (; p; p = p->next) {
        if (!strcmp(p->url, url))
            return p;
    }
    return NULL;
}

static int hash_put(struct hash_head *head, char *url) {
    unsigned hash;
    struct hash_url *p = lookup(head, url);

    if (p == NULL) {
        p = (struct hash_url*)malloc(sizeof(struct hash_url));
        if (p == NULL || (p->url = strdup(url)) == NULL)
            return -1;
        p->count = 1;
        hash = hash_num(url, SIZE);
        p->next = head->tab[hash];
        head->tab[hash] = p;
    } else {
        p->count++;
    }

    return 0;
}

static struct hash_url *hash_get(struct hash_head *head, char *url) {
    return lookup(head, url);
}

static void hash_pr(struct hash_head *head) {
    struct hash_url *p;
    int i;
    for (i = 0; i < SIZE; i++) {
        p = head->tab[i];
        for (;p; p = p->next) {
            printf("%s:%d\n", p->url, p->count);
        }
    }
    printf("\n");
}

static int hash_to_array() {
    return 0;
}

void get_index() {
    ;
}

int main(int argc, char **argv) {

#if 0
    while (*++argv) {
        printf("%d\n", hash_num(*argv, 100));
    }
#endif

    struct hash_head url;
    memset(&url, '\0', sizeof(struct hash_head));
    char *p;
    while (*++argv) {
        p = *argv;
        hash_put(&url, *argv);
    }
    struct hash_url *val = hash_get(&url, p);
    printf(":::%s:%d\n", val->url, val->count);
    hash_pr(&url);
    return 0;
}
