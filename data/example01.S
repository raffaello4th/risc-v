.section .text
.globl _start
_start:
	    lui a1,       %hi(msg)       # load msg(hi)
	    addi a1, a1,  %lo(msg)       # load msg(lo)
	    jalr ra
2:	    j 2b

.section .rodata
msg:
	    .string "Hello World\n"

