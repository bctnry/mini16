

        .8086
        .model tiny

        .code                    
                                  
_init:
        ;; FAT12 requires these fields to be this way.
        JMP boot
        NOP                                           
                  
        ;; FAT12 BIOS Parameter Block
        db 'MINI16  '
        dw 512
        ;; this boot sector is expected to write on a 1.44mb floppy drive.
        dw 1   ;; sector per cluster
        db 1   ;; # of reserved sectors
        db 1   ;; # of FAT table.
        dw 1   ;; # of directory entries.
        dw 1   ;; # of total sectors in logical volume.
        db 1   ;; media descriptor type.
        dw 1   ;; # of sectors per FAT.
        dw 1   ;; # of sectors per track.
        dd 0
        dd 0
        
        ;; extended boot record.
        db 0    ; drive number.
        db 0    ; 
        db 28h  ; signature (don't know what it does but it's in the spec)
        dd 1
        db 'MINI16_BOOT'    ; volume label
        db 'MINI16  '       ; "system identifier" string.
        

bios_reset dd 0ffff0000h
kernel dd 00001400h
boot:
        XOR AX, AX
        MOV SS, AX
        MOV SP, 7ffeh

        MOV AX, 3
        INT 10H

        ;; load kernel.
        MOV AH, 2
        MOV AL, 16              ; 16 sector 8KB
        MOV CH, 0
        MOV CL, 2
        MOV DH, 0
        
        MOV BX, 0
        MOV ES, BX
        MOV BX, 1400h

        INT 13h

        CMP BYTE PTR ES:[1400h], 0ebh
        JE chk_ok

        LEA SI, msg_fail
        CALL print_string

        MOV AH, 0
        INT 16H

reboot:
        MOV AX, 40h
        MOV DS, AX
        MOV WORD PTR DS:[72h], 0
        JMP bios_reset


chk_ok:
        JMP kernel

;;; print string using AH=0eh,INT10.
;;; input: DS:[SI] - data
print_string:
        PUSH AX
        PUSH SI
        MOV AH, 0eh
next_char:
        MOV AL, [SI]
        CMP AL, 0
        JE printed
        INC SI
        INT 10H
        JMP next_char
printed:
        POP SI
        POP AX
        RET

msg_fail:
        DB 'Integrity check failed. Press any key to reboot.', 0

        db 510-($-_init) dup (0)

        dw 0aa55h

        end
