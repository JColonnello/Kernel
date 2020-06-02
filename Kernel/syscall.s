GLOBAL syscallHandler

extern funcTable

syscallHandler:
    mov rax, [rsp + 8]
    mov rdi, funcTable
	shl rax, 3
	add rdi, rax
    call [rdi]
    ret