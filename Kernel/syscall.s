GLOBAL syscallHandler

extern funcTable

syscallHandler:
    mov rdi, funcTable
	shl rax, 3
	add rdi, rax
    call [rdi]
    ret