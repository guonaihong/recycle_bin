#include <stdio.h>
#include <string.h>

void ss(void *a, int elem_size, size_t n, 
        int (*cmp)(void *, void *));

void swap(void *a, void *b, size_t n);

int cmp_char(void *a, void *b) {
    return *(char *)a > *(char *)b;
}

int cmp_int(void *a, void *b) {
    return *(int *)a > *(int *)b;
}

int cmp_short(void *a, void *b) {
    return *(short *)a > *(short *)b;
}

void pr_int(int arr[], int n) {
    int i;
    for (i = 0; i < n; i++)
        printf("%d ", arr[i]);

    printf("\n");
}

void pr_short(short arr[], int n) {
    int i;
    for (i = 0; i < n; i++)
        printf("%hd ", arr[i]);

    printf("\n");
}
int main() {

#if 1
    char *p = "11111111111";
    char *p1 = "aaaaaaaaaaa";
    swap(&p, &p1, sizeof(char *));
    printf("%s:%s\n", p, p1);
#endif
#if 1
    char buf[] = "13478923";
    ss(&buf, 1, strlen(buf), cmp_char);
    printf("::%s\n", buf);

    int arr_i[] = {10,9,8,7,6,5,4,3};
    ss(&arr_i, 4, 8, cmp_int);
    pr_int(arr_i, 8);


    short arr_s[] = {10,9,8,7,6,5,4,3};
    ss(&arr_s, 2, 8, cmp_short);
    pr_short(arr_s, 8);
#endif
    return 0;
}

/*select sort*/
void ss(void *a, int elem_size, size_t n, 
        int (*cmp)(void *, void *)) {
    int k;
    int i, j;

    for (i = 0; i < n - 1; i++) {
        k = i;
        for (j = i + 1; j < n; j++) {
            if (cmp(a + k * elem_size, a + j * elem_size)) {
                k = j;
            }
        }

        if (k != i) {
            swap(a + k * elem_size, a + i * elem_size, elem_size);
        }
    }
}

void swap(void *a, void *b, size_t n) {
    char t;
    char *a0 = a;
    char *b0 = b;

    while (n--) {
        t = *a0;
        *a0 = *b0;
        *b0 = t;
        a0++, b0++;
    }
}
