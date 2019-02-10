.section .text
.global _start
_start:
	la a0,msg 	#将msg标签对应的地址赋值给a0寄存器

.section .rodata

msg:
	.string "Hello World\n"

