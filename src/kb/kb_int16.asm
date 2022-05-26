;;; Mini16 INT16 wrapper
;;; Normally one would write the driver himself. This is just for
;;; simplicity.

	.8086
	.model tiny
	.code

	;; public kb_get_flag_
	;; public kb_get_scan_code_
	;; public kb_get_ascii_

	;; char kb_get_flag()
	;; INT 16h AH=02h
	public kb_get_flag_
kb_get_flag_:
	MOV AH, 2
	INT 16h
	MOV AH, 0

	RET
	
	public kb_get_scan_code_
kb_get_scan_code_:
	MOV AH, 0
	INT 16h
	MOV AL, AH
	RET

	public kb_get_ascii_
kb_get_ascii_:
    public kb_get_key_
kb_get_key_:
	MOV AH, 0
	INT 16h
	RET

	public kb_check_key_
kb_check_key_:
	MOV AH, 1
	INT 16H
	JZ kb_check_key_no_key
	MOV AL, 1
	RET
kb_check_key_no_key:
	MOV AL, 0
	RET
	
	end
