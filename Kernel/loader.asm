global loader
extern main
extern _init
extern endOfKernel
extern startOfUniverse
extern bss

section .text_loader

PML4_ADDR equ 0x50000
PDP_ADDR equ 0x51000
PD_ADDR equ 0x52000
PT_ADDR equ 0x53000

loader:
	mov rcx, endOfKernel		; Calculate mapped space in pages
	sub rcx, startOfUniverse
	shr rcx, 12					; Divide by 4K
	mov rdx, rcx				; Backup count

	add rcx, 3			; Add 3 pages for PML4, PDP & PD
	xor rax, rax
	mov rdi, PML4_ADDR	; Clear tables
	rep stosq

	mov rcx, rdx
	mov rdi, PT_ADDR	; Reserve pages for kernel
	mov rax, 0xF		; Add access flags and cache pass-through		
load_PT:
	stosq
	add rax, 0x1000		; Add 4KiB
	loop load_PT

	; Add a trap page between kernel code and stack
	and rax, ~0x1			; Remove present flag
	stosq

	add rax, 0x1000 | 0x7	; Reserve stack page
	stosq
	and rax, ~0xFFF			; Remove flags
	add rax, 0x1000			; Add offset and go to end of page
	mov rsp, rax
	
	mov rdi, PD_ADDR				; Create first PD entry
	mov rax, PT_ADDR | 0x7
	stosq

	mov rdi, PDP_ADDR + 511 * 8		; Create PDP entry 512
	mov rax, PD_ADDR | 0x7
	stosq

	mov rdi, PML4_ADDR + 511 * 8	; Create PML4 entry 512
	mov rax, PDP_ADDR | 0x7
	stosq

	mov rdi, PDP_ADDR				; Create PDP entry 1
	mov rax, PD_ADDR | 0x7
	stosq

	mov rdi, PML4_ADDR				; Create PML4 entry 1
	mov rax, PDP_ADDR | 0x7
	stosq

	mov rax, PML4_ADDR | 0x8
	mov cr3, rax					; Update CR3
	add rsp, startOfUniverse		; Set stack to high half
	mov rax, longJump
	jmp rax

longJump:
	; Unmap lower mirror
	mov rdi, startOfUniverse + PDP_ADDR		; Delete PDP entry 1
	xor rax, rax
	stosq

	mov rdi, startOfUniverse + PML4_ADDR	; Delete PML4 entry 1
	xor rax, rax
	stosq

	mov rax, cr3
	mov cr3, rax

	; Point IDT descriptors to high half
	mov rcx, 256
	mov rdi, startOfUniverse
fixIDT:
	add rdi, 6
	mov ax, [rdi]
	or ax, 0xC000
	mov [rdi], ax

	add rdi, 2
	mov [rdi], dword 0xFFFFFFFF
	add rdi, 8
	loop fixIDT
	; Reload GDT & IDT
	lgdt [GDTR64]
	lidt [IDTR64]

clearBSS:
	mov rdi, bss
	mov rcx, endOfKernel
	sub rcx, rdi
	shr rcx, 3
	xor rax, rax
	rep stosq

	mov rdi, _init
	call rdi
	mov rdi, main
	call rdi
hang:
	cli
	hlt	; halt machine should kernel return
	jmp hang

IDTR64: dw 256*16-1
		dq startOfUniverse

GDTR64: dw 3 * 8 - 1
		dq startOfUniverse + 0x1000