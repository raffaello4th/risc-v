#include "rv_mem.h"
#include <memory.h>
u_int32*    init_memory32(rv32_mem* _mem,u_int32 _size)
{
    _mem->head = (u_int32*)malloc(_size * sizeof(u_int32));
    memset(_mem->head,0,_size * sizeof(u_int32));
	_mem->size = _size;
    return _mem->head;
}
void free_memory32(rv32_mem* _mem){
    free(_mem->head);
    _mem->size = 0;
}
