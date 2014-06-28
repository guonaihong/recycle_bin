#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

int value;
int status;

enum {
	ON,
	OFF,
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond =  PTHREAD_COND_INITIALIZER;

void *thread_one(void *arg) {
	//sleep(1);//test use
	pthread_mutex_lock(&mutex);

	value = 5;
	status = OFF;
	pthread_cond_signal(&cond);

	printf("%d\n", value);
	pthread_mutex_unlock(&mutex);

	return (void *)0;
}

void *thread_two(void *arg) {
	//sleep(1); //test use
	pthread_mutex_lock(&mutex);

	while (status == ON) {
		pthread_cond_wait(&cond, &mutex);
	}

	value = 6;
	printf("%d\n", value);
	pthread_mutex_unlock(&mutex);

	return (void *)0;
}

#define SIZE 2
int main() {

	pthread_t tids[SIZE] = {0};
	int rv[SIZE];
	rv[0] = pthread_create(&tids[0], NULL, thread_one, NULL);
	rv[1] = pthread_create(&tids[1], NULL, thread_two, NULL);

	if (rv[0] != 0 || rv[1] != 0) {
		fprintf(stderr, "thread_one %s thread_two %s\n",
			   	strerror(rv[0]), strerror(rv[1]));
		return 1;
	}

	pthread_join(tids[0], NULL);
	pthread_join(tids[1], NULL);
	return 0;
}
