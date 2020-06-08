
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL setupIDTHandlers
GLOBAL defaultException
GLOBAL defaultInterrupt

EXTERN irqTable
EXTERN intTable
EXTERN exceptionTable
EXTERN __startOfUniverse
EXTERN ncPrintChar
EXTERN ncPrint
EXTERN ncPrintPointer
EXTERN ncNewline


SECTION .text

%macro pushState 0
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
	pop rsp
%endmacro

%assign i 0
%rep 0x20
_exception_%[i]_Handler:
	push qword i
	jmp _exception_Handler
%assign i i+1
%endrep

_exception_Handler:
	pushState
	mov rax, [rsp + 8 * 16] ; i
	shl rax, 3
	add rax, exceptionTable
	mov rax, [rax]
	test rax, rax
	jnz .call
	mov rax, defaultException
.call:	
	call rax
	popState
	add rsp, 8	;pop i
	iretq

%assign i 0
%rep 0x30 - 0x20
_irq_%[i]_Handler:
	push qword i
	jmp _irq_Handler
%assign i i+1
%endrep

_irq_Handler:
	pushState
	mov rax, [rsp + 8 * 16] ; i
	shl rax, 3
	add rax, irqTable
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
	add rsp, 8	;pop i
	iretq

%assign i 0
%rep 0x100 - 0x30
_int_%[i]_Handler:
	push qword i
	jmp _int_Handler
%assign i i+1
%endrep

_int_Handler:
	pushState
	mov rax, [rsp + 8 * 16] ; i
	shl rax, 3
	add rax, intTable
	mov rax, [rax]
	test rax, rax
	jnz .call
	mov rax, defaultInterrupt
.call:	
	call rax
	; replace stored rax for returned value
	add rsp, 8
	push rax
	popState
	add rsp, 8	;pop i
	iretq

setupIDTHandlers:
	mov rdi, __startOfUniverse
	%assign i 0x20 - 1
	%rep 0x20
	push _exception_%[i]_Handler
	%assign i i-1
	%endrep
	mov rcx, 0x20
.loop1:
	call make_exception_gates
	pop rax
	loop .loop1

	%assign i 0x30 - 0x20 - 1
	%rep 0x30 - 0x20
	push _irq_%[i]_Handler
	%assign i i-1
	%endrep
	mov rcx,  0x30 - 0x20
.loop2:
	call make_interrupt_gates
	pop rax
	loop .loop2

	%assign i 0x100 - 0x30 - 1
	%rep 0x100 - 0x30
	push _int_%[i]_Handler
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
	mov ax, 0x8E00
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

defaultException:
	call ncNewline
	mov rdi, int_string
	call ncPrint
	mov rdi, exc_string
	mov rax, [rsp + 8*17]		; Exception number
	and rax, 0xFF			; Clear out everything in RAX except for AL
	shl rax, 3				; Quick multiply by 3
	add rdi, rax			; Use the value in RAX as an offset to get to the right message
	call ncPrint
	mov rdi, adr_string
	call ncPrint
	; Skip return address, exception number, 16 pushed regs
	mov rdi, [rsp+0x8*19]	; RIP
	call ncPrintPointer
	call ncNewline
	lea rdi, [rsp+8]		; RAX
	call os_dump_regs
	jmp _hltForever

_hltForever:
	cli
	hlt
	jmp _hltForever

_cli:
	cli
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


os_dump_regs:
	mov byte [os_dump_reg_stage], 0x00	; Reset the stage to 0 since we are starting
	mov rbx, rdi
	call ncNewline

os_dump_regs_again:
	mov rdi, os_dump_reg_string
	xor rax, rax
	xor rcx, rcx
	mov al, [os_dump_reg_stage]
	mov cl, 5				; each string is 5 bytes
	mul cl					; ax = cl x al
	add rdi, rax
	call ncPrint			; Print the register name

	mov rax, [rbx]
	add rbx, 8
	mov rdi, rax
	call ncPrintPointer

	add byte [os_dump_reg_stage], 1
	cmp byte [os_dump_reg_stage], 0x10
	jne os_dump_regs_again
	ret

os_dump_reg_string: 
	db '  A:', 0
	db '  B:', 0
	db '  C:', 0
	db '  D:', 0
	db '  8:', 0
	db '  9:', 0
	db ' 10:', 0
	db ' 11:', 0
	db ' 12:', 0
	db ' 13:', 0
	db ' 14:', 0
	db ' 15:', 0
	db ' DI:', 0
	db ' SI:', 0
	db ' BP:', 0
	db ' SP:', 0

os_dump_reg_stage: db 0x00
int_string db 'Pure64 - Exception ', 0
adr_string db ' @ 0x', 0
align 16
exc_string:
	db '00 - DE', 0
	db '01 - DB', 0
	db '02     ', 0
	db '03 - BP', 0
	db '04 - OF', 0
	db '05 - BR', 0
	db '06 - UD', 0
	db '07 - NM', 0
	db '08 - DF', 0
	db '09     ', 0		; No longer generated on new CPU's
	db '10 - TS', 0
	db '11 - NP', 0
	db '12 - SS', 0
	db '13 - GP', 0
	db '14 - PF', 0
	db '15     ', 0
	db '16 - MF', 0
	db '17 - AC', 0
	db '18 - MC', 0
	db '19 - XM', 0
