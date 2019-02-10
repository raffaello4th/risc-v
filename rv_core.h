#ifndef _RV_CORE_H_
#define _RV_CORE_H_
#include "rv_type.h"
#include "rv_mem.h"
#ifdef RV32I
    #define RV_MAX_REG_NUM  32
#elif RV32E    
    #define RV_MAX_REG_NUM  16
#else    
    #define RV_MAX_REG_NUM  32
#endif
typedef struct _rv32{
	rv32_mem* mem;
    u_int32 x[RV_MAX_REG_NUM];
    u_int32 pc;
} rv32;
#pragma pack(push) 

#pragma pack(1)
typedef union _instru32{
    u_int32 instruction;
    struct{
        u_int8  opcode : 7;
        union{
            struct _R{
                    u_int8 rd : 5;
                    u_int8 funct3 : 3;
                    u_int8 rs1 : 5;
                    u_int8 rs2 : 5;
                    u_int8 funct7 : 7;
            } R;
            struct _I{
                    u_int8 rd : 5;
                    u_int8 funct3 : 3;
                    u_int8 rs1 : 5;
                    u_int16 imm : 12;
            } I;
            struct _S{
                    u_int8  imm0 : 5;
                    u_int8 funct3 : 3;
                    u_int8 rs1 : 5;
                    u_int8 rs2 : 5;
                    u_int8 imm1 : 7;
            } S;
            struct _U{
                    u_int8 rd : 5;
                    u_int32 imm : 20;
            } U;
        };
        /*
        union {
                u_int8  rd : 5;
                u_int8  imm5 : 5;
        };
        union{
            struct {
                u_int8  funct3 : 4;
                u_int8  rs1 : 5;
                union{
                    struct{
                        u_int8 rs2 : 5;
                        u_int8 funct7 : 7;
                    };
                    u_int16 imm12 : 12;
                    struct{
                        u_int8 rs2 : 5;
                        u_int8 imm7 : 7;
                    };
                };
            };
            u_int32 imm20 : 20;
        };
        */
    };
} instru32;
typedef struct _imm12{
    union{
        u_int16 imm12 : 12;
        struct {
            u_int8 imm5 : 5;
            u_int8 imm7 : 7;
        };
    };
} imm12;
#pragma pack(pop)
rv32*   rv32_init(u_int32 _mem_size);
void    rv32_free(rv32** _rv32);
u_int32 rv32_run(rv32* _rv);
void	rv32_load(rv32* _rv, const char* _filename);
#endif//_RV_CORE_H_