global _getKeyCode
global cleanInt

_getKeyCode:
    mov rax,0
    in al,60h
    ret

cleanInt:
    mov al, 20h
	out 20h, al
    ret