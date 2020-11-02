global loader
extern main
extern _init
extern __endOfKernel
extern __startOfUniverse
extern __bss
extern exception_stack

PML4_ADDR equ 0x2000
PDP_ADDR equ 0x3000
PD_ADDR equ 0x10000
PT_ADDR equ 0x50000

GDT equ __startOfUniverse + 0x1000

section .text_loader

loader:
	mov rcx, __endOfKernel		; Calculate mapped space in pages
	mov rax, __startOfUniverse
	sub rcx, rax
	shr rcx, 12					; Divide by 4K
	mov rdx, rcx				; Backup count

	mov rcx, 512		; Clear page tables
	xor rax, rax
	mov rdi, PT_ADDR
	rep stosq

	mov rcx, rdx
	mov rdi, PT_ADDR	; Reserve pages for kernel
	mov rax, 0b1011		; Add access flags and cache pass-through		
.load_PT:
	stosq
	add rax, 0x1000		; Add 4KiB
	loop .load_PT

	; Add a trap page between kernel code and stack
	and rax, ~0x1			; Remove present flag
	stosq

	mov rcx, 16
	or rax, 0x7	; Reserve stack page
.loopStack:
	add rax, 0x1000
	stosq
	loop .loopStack

	and rax, ~0xFFF			; Remove flags
	add rax, 0x1000			; Add offset and go to end of page
	mov rsp, rax
	
	mov rdi, PD_ADDR				; Create first PD entry
	mov rax, PT_ADDR | 0x3
	stosq

	mov rdi, PDP_ADDR + 511 * 8		; Create PDP entry 512
	mov rax, PD_ADDR | 0x3
	stosq

	mov rdi, PML4_ADDR + 511 * 8	; Create PML4 entry 512
	mov rax, PDP_ADDR | 0x3
	stosq

	mov rax, PML4_ADDR | 0x8
	mov cr3, rax					; Update CR3
	mov rax, __startOfUniverse
	add rsp, rax					; Set stack to high half
	mov rax, longJump
	jmp rax

longJump:
	; Unmap lower mirror
	
	mov rcx, 511
	mov rdi, __startOfUniverse + PD_ADDR + 8	; Delete PD entries 2-512
	xor rax, rax
	rep stosq

	mov rcx, 511
	mov rdi, __startOfUniverse + PDP_ADDR		; Delete PDP entries 1-511
	xor rax, rax
	rep stosq

	mov rcx, 511
	mov rdi, __startOfUniverse + PML4_ADDR 	; Delete PML4 entry 1-511
	xor rax, rax
	rep stosq

	mov rdi, __startOfUniverse + PML4_ADDR + 510 * 8	; Create PML4 entry 511 for loopback
	mov rax, PML4_ADDR | 0x3
	stosq

	mov rax, cr3
	mov cr3, rax

	; Point IDT descriptors to high half
	mov rcx, 256
	mov rdi, __startOfUniverse
.fixIDT:
	add rdi, 6
	mov ax, [rdi]
	or ax, 0xC000
	mov [rdi], ax

	add rdi, 2
	mov [rdi], dword 0xFFFFFFFF
	add rdi, 8
	loop .fixIDT

.createTSS:
	mov rax, TSS
	lea rdi, [GDT + 16]
	mov rbx, rax		; save TSS address
	mov ax, 0x67		; segment limit
	stosw
	mov rax, rbx		; get address
	stosw				; store low word (15..0)
	shr rax, 16
	stosb				; store (23..16)
	mov al, 0b10001001	; TSS marker
	stosb
	mov al, 0			; granularity and high limit
	stosb
	mov rax, rbx
	shr rax, 24
	stosb				; store (31..24)
	shr rax, 8
	stosd				; store extra high word (63..32)
	xor rax, rax
	stosd				; reserved

	; Reload GDT & IDT
	mov rax, GDTR64
	lgdt [rax]
	mov rax, IDTR64
	lidt [rax]

	; Load TSS
	mov ax, 2 << 3 | 0b000
	ltr ax

	; Configure RTC
	mov al, 0x0B
	out 0x70, al
	in al, 0x71
	bts rax, 1
	bts rax, 2
	push rax
	mov al, 0x0B
	out 0x70, al
	pop rax
	out 0x71, al

.finish:
	mov rdi, _init
	call rdi
.hang:
	cli
	hlt	; halt machine should kernel return
	jmp .hang

IDTR64: dw 256*16-1
		dq __startOfUniverse

GDTR64: dw 2 * 16 - 1
		dq __startOfUniverse + 0x1000

TSS: 	resb 0x24
		dq exception_stack
		resb 0x3c

section .bss
align 16
resb 0x1000 * 4
exception_stack: