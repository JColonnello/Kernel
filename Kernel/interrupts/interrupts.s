
GLOBAL _cli
GLOBAL _sti
GLOBAL _cr2
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL setupIDTHandlers
GLOBAL defaultInterrupt

EXTERN defaultException
EXTERN irqTable
EXTERN intTable
EXTERN exceptionTable
EXTERN __startOfUniverse
EXTERN ncPrintChar
EXTERN ncPrint
EXTERN ncPrintPointer
EXTERN ncNewline
EXTERN exit


SECTION .text

%macro pushState 0
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
%endmacro

%macro popState 0
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
%endmacro

%macro EC_exceptionHandler 1
_exception_%1_Handler:
	push qword %1
	jmp exceptionGate
%endmacro

%macro NoEC_exceptionHandler 1
_exception_%1_Handler:
	push qword 0	; Push error code 0
	push qword %1
	jmp exceptionGate
%endmacro

NoEC_exceptionHandler 0
NoEC_exceptionHandler 1
NoEC_exceptionHandler 2
NoEC_exceptionHandler 3
NoEC_exceptionHandler 4
NoEC_exceptionHandler 5
NoEC_exceptionHandler 6
NoEC_exceptionHandler 7
EC_exceptionHandler 8
NoEC_exceptionHandler 9
EC_exceptionHandler 10
EC_exceptionHandler 11
EC_exceptionHandler 12
EC_exceptionHandler 13
EC_exceptionHandler 14
NoEC_exceptionHandler 15
NoEC_exceptionHandler 16
EC_exceptionHandler 17
NoEC_exceptionHandler 18
NoEC_exceptionHandler 19
NoEC_exceptionHandler 20
%assign i 21
%rep 30 - 21
NoEC_exceptionHandler i
%assign i i+1
%endrep
EC_exceptionHandler 30
NoEC_exceptionHandler 31

exceptionGate:
	pushState
	mov rdi, rsp

	mov rax, [rdi + 8 * 15] ; i
	shl rax, 3
	mov rbx, exceptionTable
	add rax, rbx
	mov rax, [rax]
	test rax, rax
	jnz .call
	mov rax, defaultException
.call:	
	call rax
	popState
	iretq

%assign i 0
%rep 0x30 - 0x20
_irq_%[i]_Handler:
	push qword 0	; EC stub
	push qword i
	jmp irqGate
%assign i i+1
%endrep

irqGate:
	pushState
	mov rdi, rsp

	mov rax, [rdi + 8 * 15] ; i
	shl rax, 3
	mov rbx, irqTable
	add rax, rbx
	mov rax, [rax]
	test rax, rax
	jnz .call
	mov rax, defaultInterrupt
.call:	
	call rax
	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al
	popState
	add rsp, 16	; pop i & EC
	iretq

%assign i 0
%rep 0x100 - 0x30
_int_%[i]_Handler:
	push qword 0	; EC stub
	push qword i
	jmp intGate
%assign i i+1
%endrep

intGate:
	pushState
	mov rax, [rsp + 8 * 15] ; i
	shl rax, 3
	mov rbx, intTable
	add rax, rbx
	mov rax, [rax]
	test rax, rax
	jnz .call
	mov rax, defaultInterrupt
.call:	
	push rax
	mov rax, [rsp+8]
	sub rsp, 8	; Align
	call [rsp + 8]
	add rsp, 16
	; replace stored rax for returned value
	mov [rsp], rax
	popState
	add rsp, 16	;pop i & EC
	iretq

setupIDTHandlers:
	mov rdi, __startOfUniverse
	%assign i 0x20 - 1
	%rep 0x20
	mov rax, _exception_%[i]_Handler
	push rax
	%assign i i-1
	%endrep
	mov rcx, 0x20
.loop1:
	call make_exception_gates
	pop rax
	loop .loop1

	%assign i 0x30 - 0x20 - 1
	%rep 0x30 - 0x20
	mov rax, _irq_%[i]_Handler
	push rax
	%assign i i-1
	%endrep
	mov rcx,  0x30 - 0x20
.loop2:
	call make_interrupt_gates
	pop rax
	loop .loop2

	%assign i 0x100 - 0x30 - 1
	%rep 0x100 - 0x30
	mov rax, _int_%[i]_Handler
	push rax
	%assign i i-1
	%endrep
	mov rcx, 0x100 - 0x30
.loop3:
	call make_interrupt_gates
	pop rax
	loop .loop3
	ret

make_exception_gates: 			; make gates for exception handlers
	mov rax, [rsp+8]
	push rax			; save the exception gate to the stack for later use
	stosw				; store the low word (15..0) of the address
	mov ax, 0x08
	stosw				; store the segment selector
	mov ax, 0x8E01
	stosw				; store exception gate marker
	pop rax				; get the exception gate back
	shr rax, 16
	stosw				; store the high word (31..16) of the address
	shr rax, 16
	stosd				; store the extra high dword (63..32) of the address.
	xor rax, rax
	stosd				; reserved
	ret

make_interrupt_gates: 			; make gates for the other interrupts
	mov rax, [rsp+8]
	push rax			; save the interrupt gate to the stack for later use
	stosw				; store the low word (15..0) of the address
	mov ax, 0x08
	stosw				; store the segment selector
	mov ax, 0x8F00
	stosw				; store interrupt gate marker
	pop rax				; get the interrupt gate back
	shr rax, 16
	stosw				; store the high word (31..16) of the address
	shr rax, 16
	stosd				; store the extra high dword (63..32) of the address.
	xor rax, rax
	stosd				; reserved
	ret

defaultInterrupt:
	ret

_cli:
	cli
	ret

_cr2:
	mov rax, cr2
	ret

_sti:
	sti
	ret

picMasterMask:
    mov ax, di
    out	21h,al
    ret

picSlaveMask:
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    ret
