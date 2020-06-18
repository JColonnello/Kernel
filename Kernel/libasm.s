GLOBAL cpuVendor

GLOBAL _halt
global _hlt
global _wait
global _syscall
global outb

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

outb:
	mov rdx, rdi
	mov rax, rsi
	out dx, al
	ret

_wait:
	hlt
	hlt
	ret

_hlt:
	hlt
	ret

_halt:
	hlt
	jmp _halt

_syscall:
	mov rax, 0
	int 80h
	ret