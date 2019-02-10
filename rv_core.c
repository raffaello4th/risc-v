#include "rv_core.h"
#include "rv_mem.h"
#include "rv_instruc.h"
#include <assert.h>
#include <memory.h>
#include <stdio.h>
typedef u_int32(*_call_op_fun)(rv32*, instru32*);
static u_int32 _run_empty(rv32* _rv, instru32* _instruc) {
	assert ("error instruction!" && 0);
	return 0;
}
static u_int32 _read_reg(rv32* _rv, u_int8 _index) {
	return _rv->x[_index];
}
static void _write_reg(rv32* _rv, u_int8 _index, u_int32 _value)
{
	if (_index > 0 && _index < 32)  _rv->x[_index] = _value;
}
static u_int32 _fetch_instruction(rv32* _rv) {
	return *(_rv->mem->head + _rv->pc++);
}
//创建32位无符号整数，存放立即数到rd的高20位，低20位置0
static u_int32 _run_instrucLUI(rv32* _rv, instru32* _instruc) {
	u_int32 _imm = _instruc->instruction & 0xFFFFF000;
	_write_reg(_rv, _instruc->U.rd, _instruc->instruction & 0xFFFFF000);
	return _instruc->instruction & 0xFFFFF000;
}
//创建pc的相对地址，pc+无符号立即数(偏移量)=>rd		
static u_int32 _run_instrucAUIPC(rv32* _rv, instru32* _instruc) {
	u_int8  _rd = _instruc->U.rd;
	u_int32 _imm = _instruc->instruction & 0xFFFFF000;
	_write_reg(_rv, _instruc->U.rd, _rv->pc + _imm - 1);
	return _rv->pc + _imm - 1;
}
//J类指令，立即数+pc为跳转目标，rd存放pc+4（返回地址）跳转范围为pc(+/ -)1MB
static u_int32 _run_instrucJAL(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->U.rd, _rv->pc);
	u_int32 _imm = J_IMM32(_instruc->instruction);
	_rv->pc += ( _imm -1 );
	return 0;
}
//I类指令，rs+立即数为跳转目标，rd存放pc+1（返回地址）实现远跳转
static u_int32 _run_instrucJALR(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, _rv->pc);
	u_int32 x = I_IMM32(_instruc->instruction);
	_rv->pc = _read_reg(_rv, _instruc->I.rs1) + I_IMM32(_instruc->instruction);
	return _rv->pc;
}
//BEQ
static u_int32 _run_instrucBRANCH_0x0(rv32* _rv, instru32* _instruc) {
	if(_read_reg(_rv, _instruc->S.rs1) == _read_reg(_rv, _instruc->S.rs2)) _rv->pc += (B_IMM32(_instruc->instruction) - 1 );
	return 0x0;
}
//BNE
static u_int32 _run_instrucBRANCH_0x1(rv32* _rv, instru32* _instruc) {
	if (_read_reg(_rv, _instruc->S.rs1) != _read_reg(_rv, _instruc->S.rs2)) _rv->pc += (B_IMM32(_instruc->instruction) - 1);
	return 0x1;
}
//BLT
static u_int32 _run_instrucBRANCH_0x4(rv32* _rv, instru32* _instruc) {
	if ((int32)_read_reg(_rv, _instruc->S.rs1) <(int32)_read_reg(_rv, _instruc->S.rs2)) _rv->pc += (B_IMM32(_instruc->instruction) - 1);
	return 0x4;
}
//BGE
static u_int32 _run_instrucBRANCH_0x5(rv32* _rv, instru32* _instruc) {
	if ((int32)_read_reg(_rv, _instruc->S.rs1) >= (int32)_read_reg(_rv, _instruc->S.rs2)) _rv->pc += (B_IMM32(_instruc->instruction) - 1);
	return 0x5;
}
//BLTU
static u_int32 _run_instrucBRANCH_0x6(rv32* _rv, instru32* _instruc) {
	if ((u_int32)_read_reg(_rv, _instruc->S.rs1) <(u_int32)_read_reg(_rv, _instruc->S.rs2)) _rv->pc += (B_IMM32(_instruc->instruction) - 1);
	return 0x6;
}
//BGEU
static u_int32 _run_instrucBRANCH_0x7(rv32* _rv, instru32* _instruc) {
	if ((u_int32)_read_reg(_rv, _instruc->S.rs1) >= (u_int32)_read_reg(_rv, _instruc->S.rs2)) _rv->pc += (B_IMM32(_instruc->instruction) - 1);
	return 0x7;
}
static u_int32 _run_instrucBRANCH(rv32* _rv, instru32* _instruc) {
	static _call_op_fun _call[] = {
		_run_instrucBRANCH_0x0,
		_run_instrucBRANCH_0x1,
		_run_empty,
		_run_empty,
		_run_instrucBRANCH_0x4,
		_run_instrucBRANCH_0x5,
		_run_instrucBRANCH_0x6,
		_run_instrucBRANCH_0x7
	};
	return _call[_instruc->S.funct3](_rv, _instruc);
}
/**
LB is defined analogously for 8-bit values.
*/
static u_int32 _run_instrucLOAD_0x0(rv32* _rv, instru32* _instruc) {
	return 0x0;
}
/**
LH loads a 16-bit value from memory,
then sign-extends to 32-bits before storing in rd
*/
static u_int32 _run_instrucLOAD_0x1(rv32* _rv, instru32* _instruc) {
	return 0x1;
}
/**
LW	instruction loads a 32-bit value from memory into rd.
*/
static u_int32 _run_instrucLOAD_0x2(rv32* _rv, instru32* _instruc) {
	u_int32 _value = *(_rv->mem->head + I_IMM32(_instruc->instruction));
	_write_reg(_rv, _instruc->I.rd, _value);
	return 0x2;
}
/**
LBU is defined analogously for 8-bit values.
*/
static u_int32 _run_instrucLOAD_0x4(rv32* _rv, instru32* _instruc) {
	return 0x4;
}
/**
LHU loads a 16-bit value from memory but then
zero extends to 32-bits before storing in rd.
*/
static u_int32 _run_instrucLOAD_0x5(rv32* _rv, instru32* _instruc) {
	u_int32 _value = *(_rv->mem->head + I_IMM32(_instruc->instruction));
	//_value &= 0x0000ffff;
	_write_reg(_rv, _instruc->I.rd, _value & 0x0000ffff);
	return 0x5;
}
static u_int32 _run_instrucLOAD(rv32* _rv, instru32* _instruc) {
	static _call_op_fun _call[] = {
		_run_instrucLOAD_0x0,
		_run_instrucLOAD_0x1,
		_run_instrucLOAD_0x2,
		_run_empty,
		_run_instrucLOAD_0x4,
		_run_instrucLOAD_0x5,
		_run_empty,
		_run_empty
	};
	return _call[_instruc->I.funct3](_rv, _instruc);
}
/**
SB instruction store 32-bit, 16-bit, and 8-bit values from the low bits of register
rs2 to memory.
*/
static u_int32 _run_instrucSTORE_0x0(rv32* _rv, instru32* _instruc) {
	return 0x0;
}
/**
SH instruction store 32-bit, 16-bit, and 8-bit values from the low bits of register
rs2 to memory.
*/
static u_int32 _run_instrucSTORE_0x1(rv32* _rv, instru32* _instruc) {
	return 0x1;
}
/**
SW instruction store 32-bit, 16-bit, and 8-bit values from the low bits of register
rs2 to memory.
*/
static u_int32 _run_instrucSTORE_0x2(rv32* _rv, instru32* _instruc) {
	return 0x2;
}
/**
SW, SH, and SB instructions store 32-bit, 16-bit, and 8-bit values from the low bits of register
rs2 to memory.
*/
static u_int32 _run_instrucSTORE(rv32* _rv, instru32* _instruc) {
	static _call_op_fun _call[] = {
		_run_instrucSTORE_0x0,
		_run_instrucSTORE_0x1,
		_run_instrucSTORE_0x2,
		_run_empty,
		_run_empty,
		_run_empty,
		_run_empty,
		_run_empty
	};
	return _call[_instruc->S.funct3](_rv, _instruc);
}
//ADDI,将12位有符号立即数和rs相加，溢出忽略，直接使用结果的最低32bit，并存入rd
static u_int32 _run_instrucOP_IMM_0x0(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, I_IMM32(_instruc->instruction) + _read_reg(_rv, _instruc->I.rs1));
	return 0x0;
}
static u_int32 _run_instrucOP_IMM_0x1(rv32* _rv, instru32* _instruc) {
	if ((_instruc->instruction & 0xFE000000) == 0x00000000) {
		u_int32 _sham = _instruc->I.imm;
		_write_reg(_rv, _instruc->I.rd, _read_reg(_rv, _instruc->I.rs1) << _sham);
	}
	return 0x1;
}
//SLTI,如果rs小于立即数(都是有符号整数),将rd置1,否则置0
static u_int32 _run_instrucOP_IMM_0x2(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, (int32)_read_reg(_rv, _instruc->I.rs1) < (int32)_instruc->I.imm ? 1 : 0);
	return 0x2;
}
//SLTIU,和SLTI一致，不过都是无符号数
static u_int32 _run_instrucOP_IMM_0x3(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, (u_int32)_read_reg(_rv, _instruc->I.rs1) < (u_int32)_instruc->I.imm ? 1 : 0);
	return 0x3;
}
//XORI
static u_int32 _run_instrucOP_IMM_0x4(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, (int32)_read_reg(_rv, _instruc->I.rs1) ^ (int32)_instruc->I.imm);
	return 0x4;
}
static u_int32 _run_instrucOP_IMM_0x5(rv32* _rv, instru32* _instruc) {
	switch ((_instruc->instruction & 0xFE000000))
	{
	case 0x00000000://SRLI
	{
		u_int32 _sham = _instruc->I.imm;
		_write_reg(_rv, _instruc->I.rd, _read_reg(_rv, _instruc->I.rs1) >> _sham);
	}
	break;
	case 0x40000000://SRAI
		break;
	}
	return 0x5;
}
//ORI
static u_int32 _run_instrucOP_IMM_0x6(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, (int32)_read_reg(_rv, _instruc->I.rs1) | (int32)_instruc->I.imm);
	return 0x6;
}
//ANDI
static u_int32 _run_instrucOP_IMM_0x7(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->I.rd, (int32)_read_reg(_rv, _instruc->I.rs1) & (int32)_instruc->I.imm);
	return 0x7;
}
static u_int32 _run_instrucOP_IMM(rv32* _rv, instru32* _instruc) {
	static _call_op_fun _call[] = {
		_run_instrucOP_IMM_0x0,
		_run_instrucOP_IMM_0x1,
		_run_instrucOP_IMM_0x2,
		_run_instrucOP_IMM_0x3,
		_run_instrucOP_IMM_0x4,
		_run_instrucOP_IMM_0x5,
		_run_instrucOP_IMM_0x6,
		_run_instrucOP_IMM_0x7
	};
	return _call[_instruc->I.funct3](_rv, _instruc);
}
static u_int32 _run_instrucOP_0x0(rv32* _rv, instru32* _instruc) {
	switch (_instruc->R.funct7)
	{
	case 0x00://ADD
		_write_reg(_rv, _instruc->R.rd, (int32)_read_reg(_rv, _instruc->R.rs1) + (int32)_read_reg(_rv, _instruc->R.rs2));
		break;
	case 0x20://SUB
		_write_reg(_rv, _instruc->R.rd, (int32)_read_reg(_rv, _instruc->R.rs1) - (int32)_read_reg(_rv, _instruc->R.rs2));
		break;
	}
	return 0x0;
}
//SLL
static u_int32 _run_instrucOP_0x1(rv32* _rv, instru32* _instruc) {
	return 0x1;
}
//SLT
static u_int32 _run_instrucOP_0x2(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->R.rd, (int32)_read_reg(_rv, _instruc->R.rs1) < (int32)_read_reg(_rv, _instruc->R.rs2) ? 1 : 0);
	return 0x2;
}
//SLTU
static u_int32 _run_instrucOP_0x3(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->R.rd, _read_reg(_rv, _instruc->R.rs1) < _read_reg(_rv, _instruc->R.rs2) ? 1 : 0);
	return 0x3;
}
//XOR
static u_int32 _run_instrucOP_0x4(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->R.rd, (int32)_read_reg(_rv, _instruc->R.rs1) ^ (int32)_read_reg(_rv, _instruc->R.rs2));
	return 0x4;
}
static u_int32 _run_instrucOP_0x5(rv32* _rv, instru32* _instruc) {
	switch (_instruc->R.funct7)
	{
	case 0x00://SRL
		break;
	case 0x20://SRA
		break;
	}
	return 0x5;
}
//OR
static u_int32 _run_instrucOP_0x6(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->R.rd, (int32)_read_reg(_rv, _instruc->R.rs1) | (int32)_read_reg(_rv, _instruc->R.rs2));
	return 0x6;
}
//AND
static u_int32 _run_instrucOP_0x7(rv32* _rv, instru32* _instruc) {
	_write_reg(_rv, _instruc->R.rd, (int32)_read_reg(_rv, _instruc->R.rs1) & (int32)_read_reg(_rv, _instruc->R.rs2));
	return 0x7;
}
static u_int32 _run_instrucOP(rv32* _rv, instru32* _instruc) {
	static _call_op_fun _call[] = {
		_run_instrucOP_0x0,
		_run_instrucOP_0x1,
		_run_instrucOP_0x2,
		_run_instrucOP_0x3,
		_run_instrucOP_0x4,
		_run_instrucOP_0x5,
		_run_instrucOP_0x6,
		_run_instrucOP_0x7
	};
	return _call[_instruc->R.funct3](_rv, _instruc);
}
static u_int32 _run_instrucMISC_MEM(rv32* _rv, instru32* _instruc) {
	return 0;
}
static u_int32 _run_instrucSYSTEM(rv32* _rv, instru32* _instruc) {
	return 0;
}
static u_int32 _run_step_one32(rv32* _rv) {
	instru32 _instru;
	_instru.instruction = _fetch_instruction(_rv);
	switch (_instru.opcode) {
	case LUI:
		return _run_instrucLUI(_rv, &_instru);
		break;
	case AUIPC:
		return _run_instrucAUIPC(_rv, &_instru);
		break;
	case JAL:
		return _run_instrucJAL(_rv, &_instru);
		break;
	case JALR:
		return _run_instrucJALR(_rv, &_instru);
		break;
	case BRANCH:
		return _run_instrucBRANCH(_rv, &_instru);
		break;
	case LOAD:
		return _run_instrucLOAD(_rv, &_instru);
		break;
	case STORE:
		return _run_instrucSTORE(_rv, &_instru);
		break;
	case OP_IMM:
		return _run_instrucOP_IMM(_rv, &_instru);
		break;
	case OP:
		return _run_instrucOP(_rv, &_instru);
		break;
	case MISC_MEM:
		return _run_instrucMISC_MEM(_rv, &_instru);
		break;
	case SYSTEM:
		return _run_instrucSYSTEM(_rv, &_instru);
		break;
	}
	return 0;
}
u_int32 rv32_run(rv32* _rv)
{
	while ((_rv->mem->head[_rv->pc] & 0x3) == 0x3) {
		_run_step_one32(_rv);
	}
	return 0;
}
u_int32	rv32_run_step(rv32* _rv) {
	if ((_rv->mem->head[_rv->pc] & 0x3) == 0x3)	return _run_step_one32(_rv);
	return 0;
}
rv32*   rv32_init(u_int32 _mem_size) {
	rv32* _rv32 = malloc(sizeof(rv32));
	_rv32->mem = malloc(sizeof(rv32_mem));
	init_memory32(_rv32->mem, _mem_size);
	memset(_rv32->x, 0, RV_MAX_REG_NUM * sizeof(u_int32));
	_rv32->pc = 0x00000000;
	/*
	_rv32->mcpuid = 0x00000000;
	_rv32->mimpid = 0x00000000;
	_rv32->mhartid = 0x00000000; //only one hard thread
	_rv32->mstatus = 0x00000000;
	_rv32->mtvex = 0x00000100;
	*/
	return _rv32;
}
void    rv32_free(rv32** _rv32) {
	if (*_rv32) {
		free_memory32((*_rv32)->mem);
		free(*_rv32);
		*_rv32 = NULL;
	}
}
void	rv32_load(rv32* _rv, const char* _filename)
{
	FILE *fp;
	if (fopen_s(&fp, _filename, "rb") != EINVAL) {
		fseek(fp, 0L, SEEK_END);
		unsigned int _size = ftell(fp);
		assert(_size <= _rv->mem->size * sizeof(u_int32));
		_size = _size < _rv->mem->size * sizeof(u_int32) ? _size : _rv->mem->size * sizeof(u_int32);
		fseek(fp, 0L, SEEK_SET);
		fread_s(_rv->mem->head, _rv->mem->size * sizeof(u_int32), _size, 1, fp);
		fclose(fp);
	}
}
void	rv32_dump(rv32* _rv, const char* _filename) {
	FILE *fp;
	if (fopen_s(&fp, _filename, "wb") != EINVAL) {
		fwrite(_rv->mem->head, _rv->mem->size * sizeof(u_int32), 1, fp);
		fclose(fp);
	}
}
