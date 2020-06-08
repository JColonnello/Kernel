%macro syscallDef 2
global %1
%1:
    mov rax, %2
    int 80h
    ret
%endmacro
syscallDef write, 1
syscallDef _exit, 60
syscallDef close, 3
syscallDef unlink, 87
syscallDef time, 201
syscallDef _brk, 12

global _hlt
_hlt:
    hlt
    ret