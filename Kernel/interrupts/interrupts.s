
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL setupIDTHandlers
GLOBAL defaultException
GLOBAL defaultInterrupt

EXTERN irqTable
EXTERN exceptionTable
EXTERN startOfUniverse
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

%macro irqHandlerMaster 1
	
%endmacro

%assign i 0
%rep 0x20
_exception_%[i]_Handler:
	pushState
	push qword i
	call [exceptionTable + 8 * i]
	pop rax
	popState
	iretq
%assign i i+1
%endrep

%assign i 0
%rep 256 - 32
_irq_%[i]_Handler:
	pushState
	push qword i
	call [irqTable + 8 * i]
	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al
	pop rax
	popState
	iretq
%assign i i+1
%endrep

setupIDTHandlers:
	mov rdi, startOfUniverse
	%assign i 0
	%rep 0x20

	mov rax, _exception_%[i]_Handler
	call make_exception_gates

	%assign i i+1
	%endrep

	%assign i 0
	%rep 256 - 32

	mov rax, _irq_%[i]_Handler
	call make_interrupt_gates
	
	%assign i i+1
	%endrep
	ret

make_exception_gates: 			; make gates for exception handlers
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
	mov rax, [rsp+0x8]		; Exception number
	and rax, 0xFF			; Clear out everything in RAX except for AL
	shl rax, 3				; Quick multiply by 3
	add rdi, rax			; Use the value in RAX as an offset to get to the right message
	call ncPrint
	mov rdi, adr_string
	call ncPrint
	; Skip return address, exception number, 16 pushed regs
	mov rdi, [rsp+0x8*18]	; RIP
	call ncPrintPointer
	call ncNewline
	lea rdi, [rsp+0x10]		; RAX
	call os_dump_regs
	jmp _hltForever

_hltForever:
	cli
	hlt
	jmp _hltForever

_hlt:
	sti
	hlt
	ret

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

SECTION .bss
	aux resq 1