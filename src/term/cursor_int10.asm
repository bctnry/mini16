;;; Mini16 INT10 wrapper for cursor

	.8086
	.model tiny
	.code

	;; unsigned short cursor_get_pos
	;; INT 10h; AH=03h
	;; 	BH: page number
	public cursor_get_pos_
cursor_get_pos_:
	;; get current active page.
	PUSH DX
	PUSH CX
	PUSH BX

	MOV AH, 0fh
	INT 10h

	;; get cursor position.
	MOV AH, 03h
	INT 10h
	MOV AX, DX
	
	POP BX
	POP CX
	POP DX
	
	RET

	;; void cursor_set_pos(
	;; 	char x,		// [AX]
	;; 	char y,		// [DX]
	;; )
	public cursor_set_pos_
cursor_set_pos_:
	PUSH CX
	PUSH DX
	PUSH BX

	MOV CL, AL
	MOV AH, 0fh
	INT 10h

    MOV DH, DL
	MOV DL, CL
	MOV AH, 02h
	INT 10h
	
	POP BX
	POP DX
	POP CX

	RET

	end

