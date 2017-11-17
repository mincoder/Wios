# Wios
WIOS is a work in progress operating system.
It is planned to be fully modular but come with
prepackaged software for average users.

Planned features:
 - Working terminal
 - Commands for navigating files and folders aswell as creating them.
 - A simple text editor.
 - A programming language.
 - A desktop

Completed features:
 - Bootstrap
 - Working kernel

# install & compile linux

//Install tools

*Compiler
1. Put this(https://github.com/rm-hull/barebones-toolchain) in your $HOME folder.
2. Add "$HOME/barebones-toolchain/cross/i686/bin/" to your path.
3. To check that you did everything correctly execute the command "i686-elf-gcc --version"

*VM
You can use any VM to test the os however I perfer QEMU due to that you can run a bin
and its fast to boot. To download qemu do "sudo apt-get install qemu". To run the kernel do
"qemu-system-i386 -kernel wios.bin"

Create an issue if you need any help.

//Compilation & Execution
Execute the following commands in the root directory:
linux32 i686-elf-as boot.s -o boot.o
linux32 i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
linux32 i686-elf-gcc -T linker.ld -o wios.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
qemu-system-i386 -kernel wios.bin
