ASM=nasm
CC=i686-elf-gcc
LD=i686-elf-ld

CFLAGS=-ffreestanding -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

# Kernel object files
KERNEL_OBJS = build/kernel_entry.o \
              build/kernel.o \
              build/vga.o \
              build/idt.o \
              build/isr.o \
              build/isr_setup.o \
              build/isr_handler.o \
              build/pic.o \
			  build/keyboard.o \
			  build/shell.o

all: build/os.bin

build/boot.bin: src/boot.asm
	mkdir -p build
	$(ASM) -f bin $< -o $@

build/stage2.bin: src/stage2.asm
	$(ASM) -f bin $< -o $@

build/%.o: src/%.asm
	mkdir -p build
	$(ASM) -f elf32 $< -o $@

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.bin: $(KERNEL_OBJS)
	$(LD) -T src/linker.ld -o $@ $^ --oformat binary

build/os.bin: build/boot.bin build/stage2.bin build/kernel.bin
	cat $^ > $@

clean:
	rm -rf build

run: build/os.bin
	qemu-system-i386 -drive format=raw,file=$<,index=0,if=floppy

rebuild: clean all
