# PoppyOS

Creating an OS sounds sooo hard, let's try it out :D

Just like [Marselo](https://github.com/caiodomingues/marselo) and [OrlonDB](https://github.com/caiodomingues/orlondb), **this repo is just a study and for fun**, so don't expect it to be a fully functional OS, it's just a playground for me to learn about OS development and have fun doing it.

Exactly like the other two repos, this will not be a "real" project and probably will not be finished, as that would take "infinite" time (check the other two repos, they still have a lot of tasks and will probably never be finished).

Same rules/conventions as the other two repos apply here:

- Little to no AI involvement, I want to do most of the work myself to learn as much as possible.
- You may find random PT-BR comments/code, because it's my native language and sometimes I write comments in Portuguese to express my thoughts more naturally. (But I'll try to keep it mostly in English for better readability)
- Since this is a learning project, I do it in my free time, and my home pc has Windows due to gaming, I will be using WSL2 to do the development, so expect some commands and tools to be linux-based.

# How to build

## Prerequisites

- WSL2 with Ubuntu
- NASM (`sudo apt install nasm`)
- QEMU (`sudo apt install qemu-system-x86`)
- i686-elf cross-compiler (GCC + Binutils targeting `i686-elf`)

## Build & Run
```bash
make rebuild   # clean build
make run       # run in QEMU
```

# What's implemented so far

- Two-stage bootloader (real mode → protected mode)
- GDT setup and 32-bit protected mode
- Kernel in C
- VGA text mode driver with colors
- IDT with CPU exception handlers
- PIC remapping and hardware IRQs
- Keyboard driver (US QWERTY, scancodes → ASCII)
- Basic shell with input buffer and commands (`help`, `clear`)

# Why making it 32-bit with old i386 architecture?

Simplicity and easy to find resources/docs.

# Why another OS?

Go back to the first section and read it again :D
