# Mini16

## **NOTE: Mini16 is now [SRH Syzygy](https://github.com/bctnry/SRH-Syzygy).**

16-bit x86 operating system for teaching.

(very early version. things might change drastically.)

## Build

Tested with Open Watcom v2. v1 probably work but not tested.

Make with `make all`. If you have qemu installed, start emulation with `make start`.

(Please refer to makefile for more info)

## Roadmap

(in this exact order.)

+ hard disk MBR related utilities
+ memory management
+ making use of swap partition
  + only swapping is possible because there's no support for virtual memory in 8086 real mode.
+ round-robin task scheduler
+ IPC
+ filesystem drivers
+ kernel syscall
+ libc
+ a new shell
+ devtools related stuff (probably)

## Licence

BSD-3-Clause
