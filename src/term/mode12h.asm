	.8086
	.model tiny
	.code


    public mode12_
mode12_:
    PUSH AX
    MOV AH, 0
    MOV AL, 12h
    INT 10h
    POP AX
    RET

    public mode11_
mode11_:
    PUSH AX
    MOV AH, 0
    MOV AL, 11h
    INT 10h
    POP AX
    RET

    public mode3_
mode3_:
    PUSH AX
    MOV AH, 0
    MOV AL, 3
    INT 10h
    POP AX
    RET

    end
    