GLOBAL syscallHandler
GLOBAL _switchPML4
GLOBAL _execve
GLOBAL _switch
GLOBAL _abandon
GLOBAL _dropAndLeave
extern funcTable
extern funcTableSize
extern getKernelStack
extern execve
extern dropTable

%macro pushContext 0
	push rsp
	push rbp
	push rsi
	push rdi
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdx
	push rcx
	push rbx
	push rax
    pushfq
%endmacro

%macro popContext 0
    popfq
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	pop rdi
	pop rsi
	pop rbp
	pop rsp
%endmacro

syscallHandler:
    mov rax, [rsp + 8]
    cmp rax, [funcTableSize]
    jge .ret
	shl rax, 3
	add rax, funcTable
    mov rax, [rax]
    test rax, rax
    jz .ret
    call rax
.ret:
    ret

; Change to kernel stack before rutine
_execve:
    push rbp
    call getKernelStack
    mov rbx, rsp
    mov rsp, rax
    push rbx
    call execve
    pop rsp
    pop rbp
    ret

; Only usable from kernel stack
_switchPML4:
	or rdi, 0x8
    mov cr3, rdi
    ret

_dropAndLeave:
	push rdi
	push rsi
	call getKernelStack
	pop rsi
	pop rdi
	mov rsp, rax
	push rdi
	push rsi
	call dropTable
	pop rsi
	pop rdi
	jmp _abandon

; RDI = pml4
; RSI = new rsp
; RDX = pointer to save rsp
_switch:
	pushContext
	push qword _resume

	mov [rdx], rsp
_abandon:
	or rdi, 0x8
    mov cr3, rdi
	mov rsp, rsi
	ret ; Jump to other process

_resume:
	; RIP has alredy been pop'd
	popContext
	ret