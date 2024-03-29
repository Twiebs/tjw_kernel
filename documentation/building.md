# Introduction
I'm writing this documentation two years since the last time I touched this project. I'm writing this mostly to remember how any of this worked, so some of the info in here might be wrong.

# Required Programs:

- nasm
- gcc
- objcopy
- ld
- sh
- grub2 (grub-install)
- grub-mkrescue (grub-common)
- xorriso (for grub-mkrescue)

# Compiling The Tools

The main tool that's required by the kernel is *bin_to_txt* which converts any file something that can be included into a C array.

The purpose of this tool is so that the trampoline binary can be included in to the source files.
This makes it really easy to copy the trampoline binary it's a location where the secondary processors can start running it.

There is no build script for it. *You need to compile it manually*

# Compiling The Kernel
Note: I'm writing this in 2019 barely remembering how any of this works. I'll probably have to keep updating this
as I go.

I just added a variable to the shell script at the top of build.c called _*OutputDirectory*_.
I'm doing this crazy thing where I'm editing the source files in Windows but compiling on a laptop running Linux.
I'm relying on using filesharing to compile directly from a folder. due to permissions and such I don't want to write to this folder
so I need to add this so that I can output the binaries that the build script produces to a local directory.

- Make sure nasm is installed
- Set the OutputDirectory bash variable in build.c
- make sure bin_to_txt is built and in the OutputDirectory
- sh build.c

# Running The Kernel
The main script that does this is called run.sh.
There are a few configuration variables at the top of it that need to be hardcoded.
qemu needs to be installed.

# Building and running an ISO
- Make sure `xorriso` is installed
- Make sure `nasm` is installed
- Make sure `gcc` is installed
- Build kernel tools by running `scripts/build_tools.sh`
- Build kernel by running `scripts/build_kernel.sh`
- Make ISO file by running `scripts/make_iso.sh`
- Run ISO with qemu using `scripts/run_qemu_iso.sh`

# Windows
Must use WSL2
Install qemu on windows
Add qemu to path