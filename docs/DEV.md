# DEV

## Compiling & running

Compile with Open Watcom v2. v1 should work as well, but I haven't tested.

### Workflow

+ bootsector and kernel are built according to `Makefile`(*nix) or `make.bat`(win32). There's no measure taken to keep the two files in sync, you have to update them manually when adding new source file; `mini16.lnk` and `mini16_boot.lnk` should also be updated.
+ linking with `mini16_boot.lnk` and `mini16.lnk` will only produces the binary for the bootsector and the kernel; a Python script is used to put them together into a 1.44MBytes disk image (`mkboot.py`).
+ `mkhard.py` is used to generate raw hard disk images for emulating in qemu.
+ `make start_hard` will start the emulation with hard disk mounted.


