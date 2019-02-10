#ifndef _RV_MEM_H_
#define _RV_MEM_H_
#include "rv_type.h"
typedef struct _rv32_mem{
    u_int32* head;
    u_int32  size;
} rv32_mem;
u_int32*    init_memory32(rv32_mem* _mem,u_int32 _size);
void free_memory32(rv32_mem* _mem);
#endif//_RV_MEM_H_