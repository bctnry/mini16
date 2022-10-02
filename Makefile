clean:
	rm *.o
	rm main.bin
	rm main_boot.bin
	rm main.img

clean_obj:
	rm *.o

kernel:
	wasm -0 src/init.asm
	wcc -0 -ms src/cstdlib/string.c
	wasm -0 src/kb/kb_int16.asm
	wcc -0 -ms src/kb/kb.c
	wasm -0 src/disk/disk_int13.asm
	wcc -0 -ms src/disk/disk.c
	wasm -0 src/term/cursor_int10.asm
	wasm -0 src/term/term_int10.asm
	wasm -0 src/task/int8.asm
	wasm -0 src/util/power.asm
	wcc -0 -ms src/term/vga.c
	wcc -0 -ms src/term/term.c
	wcc -0 -ms src/shell/fdisk/fdisk.c
	wasm -0 src/vesa/vesa_asm.asm
	wasm -0 src/term/mode12h.asm
	wcc -0 -ms src/fs/fs.c
	wcc -0 -ms src/shell/shell.c
	wcc -0 -ms src/task/scheduler.c
	wcc -0 -ms src/shell/parse.c
	wcc -0 -ms src/main.c
	wlink @mini16.lnk
	make clean_obj

bootloader:
	wasm -0 src/boot/boot.asm
	wlink @mini16_boot.lnk
	make clean_obj

boot_img:
	python mkboot.py

harddisk:
	python mkhard.py 1

all:
	make bootloader
	make kernel
	make boot_img
	make harddisk

start:
	qemu-system-i386 -fda main.img

start_hard:
	qemu-system-i386 -fda main.img -hda main_hard.img -hdb main_hard_1.img
