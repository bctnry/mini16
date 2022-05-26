with open('main_hard.img', 'wb') as f:
    f.write(b'\x00' * (1024 * 1024 * 16))

import sys

if len(sys.argv) >= 2:
    for x in range(1, int(sys.argv[1])+1):
        with open(f'main_hard_{x}.img', 'wb') as f:
            f.write(b'\x00' * (1024 * 1024 * 8))
