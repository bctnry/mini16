import sys

def to_chs(x):
    head = x[0]
    sector = x[1]&0b00111111
    cylinder = ((x[1]&0b11000000)<<8)|x[2]
    return (cylinder, head, sector)

def dword_to_int(x):
    return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24)

def repl(fname):
    with open(fname, 'rb') as f:
        mbr = f.read(512)
    ptable = mbr[0x1be:-2]
    ptable_list = [
        ptable[:16],
        ptable[16:32],
        ptable[32:48],
        ptable[48:]
    ]
    while True:
        prompt = input('& ').lower()
        if prompt == 'list':
            p_exist = False
            for p in ptable_list:
                if p[4]:
                    str_x = f'{"*" if p[0] >= 0x80 else " "} {hex(p[4])} CHS{to_chs(p[1:4])}~CHS{to_chs(p[5:8])} LBA({dword_to_int(p[8:12])}) SIZE={dword_to_int(p[12:16])} ({dword_to_int(p[12:16])/2} KBytes)'
                    print(str_x)
                    p_exist = True
            if not p_exist: print('No partition found.')
        elif prompt == 'exit':
            sys.exit(0)
                    


def main(fname):
    print('An FDISK ersatz. Sebastian Higgins 2022')
    print(
'''NOTES:
1.  Only works on hard disk images
2.  Assumes all hdimage has 63 sectors and sectors are 512Bytes.
3.  Due to 
''')
    
    repl(fname)

if __name__ == '__main__':
    main(sys.argv[1])
