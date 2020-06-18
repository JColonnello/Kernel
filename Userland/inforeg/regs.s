global main
extern printreg

struct:
times 16 dq 0
end equ $
bak: dq
main:
    mov [bak], rsp
    mov rsp, end
	push rax
	push rbx
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push rdi
	push rsi
	push rbp
	mov rax, [bak]
    push rax
    mov rsp, [bak]
    mov rdi, struct
    call printreg
    ret