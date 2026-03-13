ASM=nasm
CC=i686-elf-gcc
LD=i686-elf-ld

CFLAGS=-ffreestanding -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

all: build/os.bin

# Stage 1: bootloader
build/boot.bin: src/boot.asm
	mkdir -p build
	$(ASM) -f bin $< -o $@

# Stage 2: setup of protected mode
build/stage2.bin: src/stage2.asm
	$(ASM) -f bin $< -o $@

# Kernel entry: bridge ASM -> C
build/kernel_entry.o: src/kernel_entry.asm
	$(ASM) -f elf32 $< -o $@

# Kernel C
build/kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

# Links kernel_entry + kernel
build/kernel.bin: build/kernel_entry.o build/kernel.o
	$(LD) -T src/linker.ld -o $@ $^ --oformat binary

# Merge
build/os.bin: build/boot.bin build/stage2.bin build/kernel.bin
	cat $^ > $@

build/vga.o: src/vga.c
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.bin: build/kernel_entry.o build/kernel.o build/vga.o
	$(LD) -T src/linker.ld -o $@ $^ --oformat binary

build/isr.o: src/isr.asm
	$(ASM) -f elf32 $< -o $@

build/isr_setup.o: src/isr_setup.c
	$(CC) $(CFLAGS) -c $< -o $@

build/isr_handler.o: src/isr.c
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: src/idt.c
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.bin: build/kernel_entry.o build/kernel.o build/vga.o build/idt.o build/isr.o build/isr_setup.o build/isr_handler.o
	$(LD) -T src/linker.ld -o $@ $^ --oformat binary

clean: 
	rm -rf build

run: build/os.bin
	qemu-system-i386 -drive format=raw,file=$<,index=0,if=floppy

rebuild:
	make clean
	make all