# Mini16 mk.0 Architecture

(very early draft. subject to change.)

## Memory map

According to some online sources the two bit of usable consecutive memory under real mode is:

+ `0000:0500` ~ `0000:7bff`; we will call this part Part A
+ `0000:7e00` ~ `7000:ffff`; we will call this part Part B


```
0000:0500  |----------|
           |          |
           | BOOTKRNL |
0000:ffff  |          |
1000:0000  |----------|
           | USERPROG |  (64K)
1000:ffff  |----------|
           |          |
           | USERPROG |
3000:ffff  |  (128k)  |
4000:0000  |----------|
           |          |
           |          |
           | DYNA MEM |
           |          |
           |  (256k)  |
           |          |
           |          |
7000:ffff  |----------|
```

All user programs will be loaded after `1000:0000`.

### `BOOTKRNL`

The bootloader will reside in `0000:7c00`~`0000:7dff` but in Mini16 this piece of memory is reused after booting. Both the bootloader and the kernel will use 0000:fe00 ~ 0000:ffff as stack.

````
0000:0500  |----------|
0000:05ff  |    CWD   |  (256B)
0000:0600  |----------|
0000:13ff  |    ENV   |  (3.5KB)
0000:1400  |----------|
0000:6fff  |  KERNEL  |  (26KB)
0000:7000  |----------|
           |          |  (32KB)
0000:f000  |----------|  (3KB)
0000:fc00  |----------|  (512B)
0000:fe00  |----------|
0000:ffff  | KRNL STK |  (512B)
1000:0000  |----------|
````

#### Current working directory

Because we'll only be using FAT12/16 every file will have a maximum name length of 13 bytes, so a 256-byte maximum roughly equals to 20 layers of directories, which is enough for our case.

#### Environment variables

Starting from `0000:0600` to `0000:13ff` is reserved for environment variables in the format of `VAR_NAME=var_value` or `VAR_NAME=var_value1;var_value2;...;var_value_n` if `VAR_NAME` should contain several items (e.g. `PATH`), separated by `\n` (`0xa`).

## Memory map of the kernel section

<!-- 0000~00ff  256B -->
<!-- 0000~01ff  512B -->
<!-- 0000~03ff  1KB -->
<!-- 0000~07ff  2KB -->
<!-- 0000~0fff  4KB -->
<!-- 0000~1fff  8KB -->
<!-- 0000~3fff  16KB -->
<!-- 0000~ffff  64KB -->


```
0000:1400  |----------|
           | KRNLCODE |  (21KB)
0000:6800  |----------|  
           | DISKBUFF |  (4KB)
0000:7800  |----------|
           | RESERVED |  (512B)
0000:7a00  |----------|  
           |   STACK  |  (256B)
0000:7bff  |----------|
```

## ".EX" file


```
DB 'EX'         ;; magic number at header.
DW 0            ;; init 
```

## Multitasking

Mini16 has a very simple round-robin scheduler.

## Memory safety

You really can't have the "real" memory safety in 16-bit real mode so don't do direct memory addressing in user-space programs unless you understand what you're doing.

## Loading ".COM" file

Because in DOS .COM files are loaded to offset 100h so I decided to follow this convention. This leaves us 256 bytes of room for some metadata. We will not be loading DOS .COM so using DOS Program Segment Prefix is not necessary.

```
JMP 100h            ;; 3B.
DD 0                ;; 4B. address to the next process in the scheduler chain.
DD 0                ;; 4B. address to parent process (if it's a child process)
DB 0                ;; 1B. process status. 1 - READY, 2 - WAIT.
DB 'TEST    COM'    ;; 12B. file name. 12 bytes.
DB 48 DUP (0)       ;; 48B. income message buffer.
DB 48 DUP (0)       ;; 48B. output message buffer.


DB 127 DUP (0)      ;; 127B. command line arguments. 127 bytes.
```

## Mini16 syscall

Syscall in Mini16 lives in INT 90h.
