GLOBAL cpuVendor

GLOBAL _halt
global _hlt
global _wait
global _syscall
global outb
global inb

section .text
	
cpuVendor:
	push rbx
	mov eax, esi
	cpuid

	test rdi, rdi
	jz .end

	mov [rdi], eax
	mov [rdi + 4], ebx
	mov [rdi + 8], ecx
	mov [rdi + 12], edx
.end
	pop rbx
	ret

outb:
	mov rdx, rdi
	mov rax, rsi
	out dx, al
	ret

inb:
	mov rdx, rdi
	xor rax, rax
	in al, dx
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