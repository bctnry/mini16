@echo off
SET WATCOM_PREFIX=C:\WATCOM
REM taken from os
PATH %WATCOM_PREFIX%\BINNT64;%WATCOM_PREFIX%\BINNT;%PATH%
SET INCLUDE=%WATCOM_PREFIX%\H;%WATCOM_PREFIX%\H\NT;%WATCOM_PREFIX%\H\NT\DIRECTX;%WATCOM_PREFIX%\H\NT\DDK;%INCLUDE%
SET WATCOM=%WATCOM_PREFIX%
SET EDPATH=%WATCOM_PREFIX%\EDDAT
SET WHTMLHELP=%WATCOM_PREFIX%\BINNT\HELP
SET WIPFC=%WATCOM_PREFIX%\WIPFC

REM qemu.
SET QEMU_BIN_PATH=C:\Program Files\qemu
PATH %QEMU_BIN_PATH%;%PATH%

GOTO BRANCHING

:CLEAN
del *.o
del main.bin
del main_boot.bin
del main.img
EXIT /B

:CLEAN_OBJ
del *.o
EXIT /B

:KERNEL
ECHO "Building kernel"
wasm -0 .\src\init.asm -fo=init.o
wcc -0 -ms .\src\cstdlib\string.c -fo=string.o
wasm -0 .\src\kb\kb_int16.asm -fo=kb_int16.o
wcc -0 -ms .\src\kb\kb.c -fo=kb.o
wasm -0 .\src\disk\disk_int13.asm -fo=disk_int13.o
wasm -0 .\src\term\cursor_int10.asm -fo=cursor_int10.o
wasm -0 .\src\term\term_int10.asm -fo=term_int10.o
wasm -0 .\src\task\int8.asm -fo=int8.o
wasm -0 .\src\util\power.asm -fo=power.o
wcc -0 -ms .\src\term\vga.c -fo=vga.o
wcc -0 -ms .\src\term\term.c -fo=term.o
wcc -0 -ms .\src\task\scheduler.c -fo=scheduler.o
wcc -0 -ms .\src\shell\shell.c -fo=shell.o
wcc -0 -ms .\src\shell\parse.c -fo=parse.o
wcc -0 -ms .\src\main.c -fo=main.o
wlink @mini16.lnk
CALL :CLEAN_OBJ
EXIT /B

:BOOTLOADER
ECHO "Building bootloader"
wasm -0 .\src\boot\boot.asm -fo=boot.o
wlink @mini16_boot.lnk
EXIT /B

:BOOT_IMG
python mkboot.py
EXIT /B

:HARDDISK
python mkhard.py 1
EXIT /B

:ALL
CALL :BOOTLOADER
CALL :KERNEL
CALL :BOOT_IMG
CALL :HARDDISK
EXIT /B

:START
qemu-system-i386 -fda main.img
EXIT /B

:START_HARD
qemu-system-i386 -fda main.img -hda main_hard.img -hdb main_hard_1.img
EXIT /B


:BRANCHING
IF "%1" == "clean" CALL :CLEAN
IF "%1" == "kernel" CALL :KERNEL
IF "%1" == "bootloader" CALL :BOOTLOADER
IF "%1" == "boot_img" CALL :BOOT_IMG
IF "%1" == "harddisk" CALL :HARDDISK
IF "%1" == "all" CALL :ALL
IF "%1" == "start" CALL :START
IF "%1" == "start_hard" CALL :START_HARD

