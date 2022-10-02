    .8086
    .model tiny
    .code

    
	;; unsigned short vesa_check_vbe(
    ;;  VBEInfoStructure* s   // [AX]
	;; )
    public vesa_check_vbe_
vesa_check_vbe_:
    PUSH BP
    MOV BP, SP

    PUSH BX
    MOV BX, AX

    PUSH ES
    MOV AX, 4f00h
    PUSH CS
    POP ES
    MOV DI, BX
    INT 10H

    POP ES
    POP BX

    MOV SP, BP
    POP BP
    RET

    end
