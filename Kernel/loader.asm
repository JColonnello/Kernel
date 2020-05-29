global loader
extern main
extern initializeKernelBinary

section .text_loader
loader:
	mov edi, 0x00002FF8		; Create PML4 entry 512
	mov eax, 0x00003007
	stosd

	mov edi, 0x00003FF8		; Create PDP entry 512
	mov eax, 0x00010007
	stosd

	mov edi, 0x00003FF8		; Reserve first page
	mov eax, 0x0000008F
	stosd

	mov rax, cr3
	mov cr3, rax

	mov rdi, initializeKernelBinary
	call rdi	; Set up the kernel binary, and get thet stack address
	mov rsp, rax				; Set up the stack with the returned address
	mov rdi, main
	call rdi
hang:
	cli
	hlt	; halt machine should kernel return
	jmp hang
