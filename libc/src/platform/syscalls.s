%macro syscallDef 2
global $%1
$%1:
    mov rax, %2
    int 80h
    ret

%endmacro
syscallDef write, 1
syscallDef _exit, 60
syscallDef open, 2
syscallDef close, 3
syscallDef unlink, 87
syscallDef date, 401
syscallDef _brk, 12
syscallDef execve, 59
syscallDef yield, 64
syscallDef getpid, 39
syscallDef read, 0
syscallDef getcpuinfo, 402
syscallDef temp, 400
syscallDef dumpregs, 403
syscallDef ps, 404
syscallDef memuse, 405
syscallDef kill, 62

global _hlt
_hlt:
    hlt
    ret