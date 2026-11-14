#include "lib1.h"

long long ComputeChunkSum(const struct RangeParams *params) {
    long long current_total = 0; // для текущего потока
    for (uint32_t offset = params->from_pos; offset < params->to_pos; offset++) {
        current_total += params->base_ptr[offset];
    }



    
    return current_total; 
}