GLOBAL syscallHandler
GLOBAL _switchPML4
GLOBAL execve
GLOBAL _switch
GLOBAL _dropAndLeave
GLOBAL temp
extern funcTable
extern funcTableSize
extern getKernelStack
extern freeKernelStack
extern _execve
extern dropTable
extern Scheduler_SwitchNext
extern Scheduler_Enable
extern checkProcessSignals

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
	;space for fxsave and rsp before alignment
	lea rax, [rsp - 512 - 8]
	and rax, ~0xF
	fxsave [rax]
	mov [rax + 512], rsp
	mov rsp, rax
%endmacro

%macro popContext 0
	fxrstor [rsp]
	add rsp, 512
	pop rsp
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
	mov r10, funcTableSize
    cmp rax, [r10]
    jge .ret
	shl rax, 3
	mov r10, funcTable
	add rax, r10
    mov rax, [rax]
    test rax, rax
    jz .ret
	sub rsp, 8	; Align
    call rax
	add rsp, 8
.ret:
    ret

; Change to kernel stack before rutine
execve:
	pushfq
    push rbp
	push rdi
	push rsi
	push rdx
	call getKernelStack
	pop rdx
	pop rsi
	pop rdi

    mov rcx, rsp
    mov rsp, rax
	mov rbp, rax
    push rcx
	sub rsp, 8

	cli
    call _execve

	add rsp, 8
    pop rsp
	mov rdi, rbp
	push rax
	call freeKernelStack
	pop rax
    pop rbp
	popfq
    ret

; Only usable from kernel stack
_switchPML4:
	or rdi, 0x8
    mov cr3, rdi
    ret

_dropAndLeave:
	mov rax, drop_stack
	mov rbp, rax
	mov rsp, rax
	call dropTable
	call Scheduler_SwitchNext
	call Scheduler_Enable
	cli	; Shouldn't return. Trap for future debugging
	hlt

; RDI = pml4
; RSI = pointer to new rsp
; RDX = pointer to save rsp
_switch:
	pushContext
	mov rax, _resume
	push rax

	mov [rdx], rsp
.abandon:
	mov rax, cr3
	and rax, ~0xFFF
	cmp rax, rdi
	je .skipPML4
	or rdi, 0x8
    mov cr3, rdi
.skipPML4:
	mov rsp, [rsi]
	call checkProcessSignals
	sti
	ret ; Jump to other process

_resume:
	; RIP has alredy been pop'd
	popContext
	sti
	ret

temp:
	push rbx
	; Get MSR_TEMPERATURE_TARGET
	mov ecx, 418

	;rdmsr
	; Mock data. Remove for use outside of QEMU
	mov edx, 0
	mov eax, 0x641400

	shr eax, 16
	and eax, 0xFF
	mov bl, al
	mov [rsi], bl

	; Get MSR_TEMPERATURE_TARGET
	mov ecx, 412

	;rdmsr
	; Mock data. Remove for use outside of QEMU
	mov edx, 0
	mov eax, 0x884e0000

	shr eax, 16
	and eax, 0x7F
	sub bl, al
	mov [rdi], bl

	pop rbx
	ret

section .bss
	align 16
	resb 0x1000
drop_stack: