ASM=nasm
CC=i686-elf-gcc
LD=i686-elf-ld

INCLUDES=-Isrc/cpu -Isrc/drivers -Isrc/mem -Isrc/task -Isrc/fs -Isrc/shell -Isrc/lib
CFLAGS=-ffreestanding -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs $(INCLUDES)

KERNEL_OBJS = build/kernel_entry.o \
              build/kernel.o \
              build/vga.o \
              build/idt.o \
              build/isr.o \
              build/isr_setup.o \
              build/isr_handler.o \
              build/pic.o \
              build/keyboard.o \
              build/shell.o \
              build/paging.o \
              build/pmm.o \
              build/heap.o \
              build/timer.o \
              build/task.o \
              build/switch.o \
              build/ata.o \
              build/fs.o

all: build/os.bin

build/boot.bin: src/boot/boot.asm
	mkdir -p build
	$(ASM) -f bin $< -o $@

build/stage2.bin: src/boot/stage2.asm
	$(ASM) -f bin $< -o $@

# Assembly objects
build/kernel_entry.o: src/kernel/kernel_entry.asm
	mkdir -p build
	$(ASM) -f elf32 $< -o $@

build/isr.o: src/cpu/isr.asm
	$(ASM) -f elf32 $< -o $@

build/switch.o: src/task/switch.asm
	$(ASM) -f elf32 $< -o $@

# C objects - explicit rules per subfolder
build/kernel.o: src/kernel/kernel.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/cpu/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/drivers/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/mem/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/task/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/fs/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/shell/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.bin: $(KERNEL_OBJS)
	$(LD) -T src/linker.ld -o $@ $^ --oformat binary

build/os.bin: build/boot.bin build/stage2.bin build/kernel.bin
	cat $^ > $@

build/disk.img:
	dd if=/dev/zero of=$@ bs=1024 count=1024

clean:
	rm -rf build

run: build/os.bin build/disk.img
	qemu-system-i386 -drive format=raw,file=build/os.bin,index=0,if=floppy \
	                 -drive format=raw,file=build/disk.img,index=0,if=ide

rebuild: clean all
