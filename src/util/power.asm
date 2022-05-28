;; Mini16 power utilities
;; mostly APM driver. taken from:
;;     https://wiki.osdev.org/APM

    .8086
    .model tiny

    .code

bios_reset DD 0ffff0000h

    ;; void power_reboot()
    public power_bios_reboot_
power_bios_reboot_:
    MOV AX, 40h
    MOV DS, AX
    MOV WORD PTR DS:[72h], 0
    JMP bios_reset


    ;; FAILURE_AT_ZERO unsigned short power_apm_chk()
    public power_apm_chk_
power_apm_chk_:
    MOV AH, 53h
    MOV AL, 0
    XOR BX, BX
    INT 15h
    JC power_apm_chk_error
    RET
power_apm_chk_error:
    XOR AX, AX
    RET


    ;; FAILURE_AT_ZERO char power_apm_connect()
    public power_apm_connect_
power_apm_connect_:
    MOV AH, 53h
    MOV AL, 1    ;; 01h: real mode interface
    XOR BX, BX
    INT 15h
    JC power_apm_connect_error
    MOV AX, 1
    RET
power_apm_connect_error:
    XOR AX, AX
    RET

    ;; FAILURE_AT_NON_ZERO char power_apm_disconnect()
    public power_apm_disconnect_
power_apm_disconnect_:
    MOV AH, 53h
    MOV AL, 4
    XOR BX, BX
    INT 15h
    JC power_apm_disconnect_error
power_apm_disconnect_ret:
    XOR AX, AX
    RET
power_apm_disconnect_error:
    ;; NOTE:
    ;; 1.  error code is at AH
    ;; 2.  03h means there was no connected interface so it's not an error.
    CMP AH, 03h
    JZ power_apm_disconnect_ret
    MOV AL, AH
    XOR AH, AH
    RET


    ;; FAILURE_AT_NON_ZERO char power_apm_setver(
    ;;     // NOTE: if we pass `ver` as two separate args it'll
    ;;     // take up two 16-bit reg with openwatcom. might as well
    ;;     // use one to simplify stuff, we can directly uses AH
    ;;     // and AL in asm here anyway.
    ;;     unsigned short ver,
    ;; )
    public power_apm_setver_
power_apm_setver_:
    MOV DX, AX
    MOV CH, AH
    MOV CL, AL
    MOV AH, 53h
    MOV AL, 0eh
    XOR BX, BX
    INT 15h
    JC power_apm_setver_error
    ;; NOTE: we check if it's set correctly here.
    CMP AL, DL
    JNZ power_apm_setver_ver_error
    CMP AH, DH
    JNZ power_apm_setver_ver_error
    XOR AX, AX
    RET
power_apm_setver_ver_error:
    ;; NOTE: don't know if this err code is taken by the APM spec.
    ;; fix this if it is.
    MOV AX, 0ffh
    RET
power_apm_setver_error:
    MOV AL, AH
    XOR AH, AH
    RET

    ;; FAILURE_AT_NON_ZERO char power_apm_enable_all()
    public power_apm_enable_all_
power_apm_enable_all_:
    MOV AH, 53h
    MOV AL, 08h
    MOV BX, 1
    MOV CX, 1
    INT 15h
    JC power_apm_enable_all_error
    XOR AX, AX
    RET
power_apm_enable_all_error:
    MOV AX, 1
    RET


    ;; FAILURE_AT_NON_ZERO char power_apm_set_state(
    ;;     unsigned char state
    ;; )
    public power_apm_set_state_
power_apm_set_state_:
    MOV CX, AX
    MOV AH, 53h
    MOV AL, 07h
    MOV BX, 1
    INT 15h
    JC power_apm_set_state_error
    XOR AX, AX
    RET
power_apm_set_state_error:
    MOV AX, 1
    RET

    end
