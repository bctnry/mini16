with open('main_boot.bin', 'rb') as f:
    boot_data = f.read()

with open('main.bin', 'rb') as f:
    main_data = f.read()

with open('main.img', 'wb') as f:
    f.write(boot_data)
    f.write(main_data)
    f.write(b'\x00' * (2 * 80 * 18 * 512 - len(boot_data) - len(main_data)))
