#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long long total_result = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

long long k;
long long mod;
int pnum;

void* run_thread(void* arg) {
    int thread_id = *(int*)arg;

    long long step = k / pnum;
    long long start = 1 + thread_id * step;
    long long end;


    if (thread_id == pnum - 1) {
        end = k;
    } else {
        end = start + step - 1;
    }

    long long local_res = 1;
    for (long long i = start; i <= end; i++) {
        local_res = (local_res * i) % mod;
    }

    pthread_mutex_lock(&mut);
    total_result = (total_result * local_res) % mod;
    pthread_mutex_unlock(&mut);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("НАДА ТАК %s <k> <pnum> <mod>\n", argv[0]);
        return 1;
    }

    k = atoll(argv[1]);
    pnum = atoi(argv[2]);
    mod = atoll(argv[3]);

    pthread_t threads[pnum];
    int thread_ids[pnum];

    
    // потоки
    for (int i = 0; i < pnum; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, run_thread, &thread_ids[i]);
    }

    // ожидание
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mut);

    printf("(%lld! mod %lld) = %lld\n", k, mod, total_result);
    return 0;
}