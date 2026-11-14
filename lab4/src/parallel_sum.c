#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h> 
#include <getopt.h>  
#include <sys/time.h> 
#include "utils.h"   
#include "lib1.h"   


void *ThreadWorker(void *context) {
    struct RangeParams *range = (struct RangeParams *)context;
    long long *res_ptr = malloc(sizeof(long long));
    *res_ptr = ComputeChunkSum(range);
    // возвращаем указатель на результат в главный поток
    return (void *)res_ptr;
}

int main(int argc, char **argv) {
    uint32_t threads_num = 0; // количество потоков
    uint32_t array_size = 0;  // общий размер массива
    uint32_t seed = 0;        
    while (true) {
        static struct option long_opts[] = {
            {"threads_num", required_argument, 0, 0},
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {0, 0, 0, 0}
        };
        int opt_idx = 0;
        int opt_val = getopt_long(argc, argv, "", long_opts, &opt_idx);
        if (opt_val == -1) break;

        switch (opt_val) {
            case 0:
                switch (opt_idx) {
                    case 0:
                        threads_num = (uint32_t)atoi(optarg);
                        break;
                    case 1:
                        seed = (uint32_t)atoi(optarg);  
                        break;
                    case 2:
                        array_size = (uint32_t)atoi(optarg);
                        break;
                }
                break;
            default:
                printf("unknown argumemen \n");
                return 1;
        }
    }

    if (threads_num == 0 || array_size == 0 || seed == 0) {
        printf("usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
        return 1;
    }

    int *numbers = malloc(sizeof(int) * array_size);
    GenerateArray(numbers, (int)array_size, (int)seed);
    if (threads_num > array_size) {
        threads_num = array_size;
    }

    pthread_t workers[threads_num];              // массив потоков
    struct RangeParams task_configs[threads_num]; // параметры отрезков для каждого потока
    // размер блока для кадого потока
    uint32_t chunk_length = array_size / threads_num;
    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, NULL);


    for (uint32_t k = 0; k < threads_num; k++) {
        task_configs[k].base_ptr = numbers;
        task_configs[k].from_pos = k * chunk_length;
        //  остаток
        task_configs[k].to_pos = (k == threads_num - 1) ? array_size : (k + 1) * chunk_length;
        // запуск потока с функцией ThreadWorker и передачей параметров отрезка
        if (pthread_create(&workers[k], NULL, ThreadWorker, (void *)&task_configs[k]) != 0) {
            perror("thread creation failed");
            free(numbers);
            return 1;
        }
    }
    long long final = 0;
    for (uint32_t k = 0; k < threads_num; k++) {
        long long *partial_output = NULL;
        // pthread_join блокирует выполнение пока поток не завершится
        pthread_join(workers[k], (void **)&partial_output);
        // забираем посчитанный кусок суммы и освобождаем выделенную потоком память
        if (partial_output != NULL) {
            final += *partial_output;
            free(partial_output);
        }
    }

    gettimeofday(&tv_end, NULL);
    double duration_ms = (tv_end.tv_sec - tv_start.tv_sec) * 1000.0;
    duration_ms += (tv_end.tv_usec - tv_start.tv_usec) / 1000.0;
    printf("total: %lld\n", final);
    printf("time: %f ms\n", duration_ms);
    return 0;
}