;;; Mini16 INT10 wrapper for terminal

	.8086
	.model tiny
	.code

	;; void term_echo(
	;;     char x                  // [AX]
	;; INT 10h AH=0Eh:
	;;     AL=n
	;;     CH=cylinder
	;;     CL=sector
	;;     DH=head
	;;     DL=drive
	;;     ES:BX=buf
    public term_echo_
term_echo_:
	PUSH BP
	MOV BP, SP

	PUSH BX
	MOV BL, AL

	;; get current active page.
	MOV AH, 0fh
	INT 10h

	;; write the character.
	MOV AH, 0eh
	MOV AL, BL
	INT 10h
	
	POP BX
	
	MOV SP, BP
	POP BP
	RET


	end
