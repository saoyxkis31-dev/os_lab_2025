#include <stdio.h>
#include <pthread.h>
#include <unistd.h> 


pthread_mutex_t lock_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_b = PTHREAD_MUTEX_INITIALIZER;


void* thread_one(void* arg) {
    printf("поток 1   БЕРЕТ A\n");
    pthread_mutex_lock(&lock_a);
    printf("поток 1  ВЗЯЛ А\n");
    sleep(2);


    printf("поток 1  ПЫТАЕТСЯ ЗАХВАТИТЬ Б\n");
    pthread_mutex_lock(&lock_b);
    printf(" Поток 1 ВЗЯЛ Б\n");


    pthread_mutex_unlock(&lock_b);
    pthread_mutex_unlock(&lock_a);
    return NULL;
}


void* thread_two(void* arg) {
    printf("поток 2    БЕРЕТ B\n");
    pthread_mutex_lock(&lock_b);
    printf("поток 2   ВЗЯЛ B\n");
    sleep(1);


    printf("поток 2   ПЫТАЕТСЯ ЗАХВАТИТЬ A\n");
    pthread_mutex_lock(&lock_a); 
    printf("поток 2   ВЗЯЛ A\n");


    pthread_mutex_unlock(&lock_a);
    pthread_mutex_unlock(&lock_b);
    return NULL;
}

int main() {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread_one, NULL);
    pthread_create(&t2, NULL, thread_two, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("ВСЕ ГУУУУУУУУУУУУУУУУУУУД\n");
    return 0;
}