#ifndef LIB1_H
#define LIB1_H

#include <stdint.h>

//структура параметров диапазона для потока
struct RangeParams {
    const int *base_ptr; //указатель на исходный массив с числами
    uint32_t from_pos;   //тартовый индекс для подсчет
    uint32_t to_pos;     //конечный индек
};

long long ComputeChunkSum(const struct RangeParams *params);

#endif