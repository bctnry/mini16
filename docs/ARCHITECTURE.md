# Mini16 mk.0 Architecture

(very early draft. subject to change.)

## Memory map

According to some online sources the two bit of usable consecutive memory under real mode is:

+ `0000:0500` ~ `0000:7bff`;
+ `0000:7e00` ~ `7000:ffff`;

`0000:7c00`~`0000:7e00` is the boot sector part, which can be reused later.

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
0000:05ff  |   DMRT   |  (256B)
0000:0600  |----------|
           |   CWD    |  (4KB)
0000:1600  |----------|
           |          |  (2KB)
0000:2000  |----------|
0000:cfff  |  KERNEL  |  (44KB)
0000:d000  |----------|
           | DISKBUFF |  (8KB)
0000:f000  |----------|
0000:fdff  | RESERVED |  (3.5KB)
0000:fe00  |----------|
0000:ffff  | KRNL STK |  (512B)
1000:0000  |----------|
````

#### DMRT: Drive Mounting Record Table

DMRT contains 256 record slots, each record contains a single one-byte "file system id".

File system id table:

+ 0x00 - Unspecified
+ 0x01 - FAT12
+ 0x02 - FAT16

#### Disk buffer

```
0000:d000 |------------|
0000:d1ff |  1ST SEC.  |
0000:d200 |------------| 
0000:d3ff |  DIR SEC.  |
0000:d400 |------------|
0000:d5ff | CLUS. SEC. |
0000:d600 |------------|
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
