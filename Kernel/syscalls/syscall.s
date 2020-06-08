GLOBAL syscallHandler

extern funcTable
extern funcTableSize

syscallHandler:
    mov rax, [rsp + 8]
    cmp rax, [funcTableSize]
    jge .ret
	shl rax, 3
	add rax, funcTable
    mov rax, [rax]
    test rax, rax
    jz .ret
    call rax
.ret:
    ret