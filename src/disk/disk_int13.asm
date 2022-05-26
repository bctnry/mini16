;;; Mini16 INT13 wrapper
;;; Normally one would write the driver himself. This is just for
;;; simplicity.

	.8086
	.model tiny
	.code

	public read_sector_, write_sector_
	extern disp_byte_: near ptr
	extern disp_word_: near ptr
	extern term_echo_: near ptr


	;; char get_diskop_status(
	;; 	char drive	// [AX]
	;; )
	;; INT 13h AH=01h:
	;; 	DL=drive
	;; output:
	;; 	AH=status
	public get_diskop_status_
get_diskop_status_:
	PUSH BP
	MOV BP, SP

	PUSH DX

	MOV AH, 1
	MOV DL, AL
	INT 13h

	MOV AL, AH
	XOR AH, AH

	POP DX
	
	MOV SP, BP
	POP BP
	RET

	;; char get_drive_param(
	;; 	char drive,		// [AX]
	;; 	DriveParameter* param	// [DX]
	;; )
	public get_drive_param_
get_drive_param_:
	PUSH BP
	MOV BP, SP

	PUSH SI
	PUSH CX
	PUSH DX
	MOV SI, DX
	MOV DL, AL
	MOV AH, 08h
	INT 13H
	JC get_drive_param_err

	;;;; ;; maximum head number.
	MOV [SI], DH
	;;;; ;; maximum sector number.
	MOV DL, CL
	AND CL, 00111111b
	MOV [SI+1], CL
	;;;; ;; maximum cylinder number.
	MOV DH, 0
	MOV CL, 6
	SHR DX, CL
	MOV CL, CH
	MOV CH, DL
	;; NOTE: due to alignment we have to put it into +4&+5
	MOV [SI+3], CH
	MOV [SI+2], CL
	JMP get_drive_param_exit

get_drive_param_err:
	MOV AL, AH
	MOV AH, 0
get_drive_param_exit:
	POP DX
	POP CX
	POP SI

	MOV SP, BP
	POP BP
	RET
	

	;; char read_sector(
	;;     char far* buf,            // [DX] [AX]
	;;     unsigned char n,          // [BX]
	;;     unsigned char drive,      // [CX]
	;;     unsigned char head,       // [BP+4]
	;;     unsigned short cylinder,  // [BP+6] [BP+7]
	;;     unsigned char sector      // [BP+8]
	;; )
	;; INT 13h AH=02h:
	;;     AL=n
	;;     CH=cylinder
	;;     CL=sector
	;;     DH=head
	;;     DL=drive
	;;     ES:BX=buf
read_sector_:
	PUSH BP
	MOV BP, SP

	MOV BYTE PTR [BP-1], BL

	;; ES:BX <-- DX:AX
	MOV BX, AX
	MOV ES, DX

	;; AL <-- Number of sector
	MOV AL, BYTE PTR [BP-1]

	;; AH <-- 02H
	MOV AH, 02H
	
	;; CL <-- high 2 bits of cylinder + sector.
	;; DL <-- drive number	
	MOV DH, 0
	MOV DL, BYTE PTR [BP+7]
	PUSH CX
	MOV CL, 6
	SHL DX, CL
	POP CX
	MOV DH, DL
	MOV DL, CL
	MOV CL, BYTE PTR [BP+8]
	OR CL, DH

	;; CH <-- Low 8 bits of cylinder
	MOV CH, BYTE PTR [BP+6]
	;; DH <-- head number
	MOV DH, BYTE PTR [BP+4]
	
	INT 13H
	MOV SP, BP
	POP BP
	RET

	;; void write_sector(
	;;     char far* buf,             // [DX] [AX]
	;;     unsigned char n,           // [BX]
	;;     unsigned char drive,       // [CX]
	;;     unsigned char head,        // [BP+4]
	;;     unsigned short cylinder,   // [BP+6] [BP+7]
	;;     unsigned char sector       // [BP+8]
	;; )
	;; INT 13h AH=0Eh:
	;;     AL=n
	;;     CH=cylinder
	;;     CL=sector
	;;     DH=head
	;;     DL=drive
	;;     ES:BX=buf
write_sector_:
	PUSH BP
	MOV BP, SP

	MOV BYTE PTR [BP-1], BL
	;; ES:BX <-- DX:AX
	MOV ES, DX
	MOV BX, AX

	;; AL <-- number of sector
	MOV AL, BYTE PTR [BP-1]

	;; AH <-- 03H
	MOV AH, 03H

	;; CL <-- high 2 bits of cylinder + sector
	MOV BYTE PTR [BP-1], CL	; temp0 = drive
	MOV CL, BYTE PTR [BP+8]
	MOV DH, 0
	MOV DL, BYTE PTR [BP+7]
	PUSH CX
	MOV CL, 6
	SHL DX, CL
	POP CX
	OR CL, DL

	;; DL <-- drive number
	MOV DL, BYTE PTR [BP-1]

	;; CH <-- Low 8 bits of cylinder
	MOV CH, BYTE PTR [BP+6]

	;; DH <-- head number
	MOV DH, BYTE PTR [BP+4]

	INT 13H	

	MOV SP, BP
	POP BP
	RET

	end
