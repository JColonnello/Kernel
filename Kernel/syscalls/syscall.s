GLOBAL syscallHandler

extern funcTable

syscallHandler:
    mov rax, [rsp + 8]
	shl rax, 3
	add rax, funcTable
    mov rax, [rax]
    test rax, rax,
    jz .ret
.ret:
    ret