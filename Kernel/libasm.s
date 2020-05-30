GLOBAL cpuVendor

GLOBAL _halt
global _syscall

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

_halt:
	hlt
	jmp _halt

_syscall:
	mov rax, 1
	int 80h
	ret