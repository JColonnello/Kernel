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
syscallDef ispidrun, 406
syscallDef setjobstatus, 407
syscallDef block, 408
syscallDef sem_wait, 409
syscallDef sem_release, 410
syscallDef sem_create, 411
syscallDef sem_getId, 412
syscallDef sem_open, 413
syscallDef sem_close, 414
syscallDef pipe, 415
syscallDef dup, 416
syscallDef processPriority, 417

global _hlt
_hlt:
    hlt
    ret

global cpuid
cpuid:
	push rbx
	mov eax, esi
	cpuid

	test rdi, rdi
	jz .end

	mov [rdi], eax
	mov [rdi + 4], ebx
	mov [rdi + 8], ecx
	mov [rdi + 12], edx
.end
	pop rbx
	ret