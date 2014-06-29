#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>

#define TSIZE 2
pthread_mutex_t mutex[TSIZE];

void *thread_one(void *arg) {
    pthread_mutex_lock(&mutex[0]);
    sleep(1);
    pthread_mutex_lock(&mutex[1]);
    printf("go %s\n", __func__);
    pthread_mutex_unlock(&mutex[0]);
    pthread_mutex_unlock(&mutex[1]);
    return (void *)0;
}

void *thread_two(void *arg) {
    sleep(1);
    pthread_mutex_lock(&mutex[1]);
    pthread_mutex_lock(&mutex[0]);
    printf("go %s\n", __func__);
    pthread_mutex_unlock(&mutex[1]);
    pthread_mutex_unlock(&mutex[0]);
    return (void *)0;
}

int main() {
    int rv[TSIZE];
    pthread_t tid[TSIZE];

    rv[0] = pthread_create(&tid[0], NULL, thread_one, NULL);
    rv[1] = pthread_create(&tid[1], NULL, thread_two, NULL);

    if (rv[0] != 0 || rv[1] != 0) {
        printf("one thread = %s:two thread = %s",
                strerror(rv[0]), strerror(rv[1]));
        return -1;
    }
    memset(rv, 0, sizeof(rv));
    rv [0] = pthread_mutex_init(&mutex[0], NULL);
    rv [1] = pthread_mutex_init(&mutex[1], NULL);
    if (rv[0] != 0 || rv[1] != 0) {
        printf("one thread = %s:two thread = %s",
                strerror(rv[0]), strerror(rv[1]));
        return -1;
    }
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    pthread_mutex_destroy(&mutex[0]);
    pthread_mutex_destroy(&mutex[1]);
    return 0;
}
