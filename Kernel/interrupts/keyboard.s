global _getKeyCode

_getKeyCode:
    mov rax,0
    in al,60h
    ret