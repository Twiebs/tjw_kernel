#Introduction
I'm writing this documentation two years since the last time I touched this project. 
I'm writing this mostly to remember how the hell this shit worked, so some of the info in here might be wrong.

#RequiredPrograms:
– nasm
– gcc
– objcopy
– ld
– sh

#CompilingTheTools
the main tool that's required by the kernel is *bin_to_txt* which converts any file something that can be included into a C array.
the purpose of this tool is so that the trampoline binary can be included in to the source files.
This makes it really easy to copy the trampoline binary it's a location where the secondary processors can start running it.
there is no build script for it. *You need to compile it manually*

#CompilingTheKernel
Note: I'm writing this in 2019 barely remembering how any of this works. I'll probably have to keep updating this
as I go.

I just added a variable to the shell script at the top of build.c called _*OutputDirectory*_.
I'm doing this crazy thing where I'm editing the source files in Windows but compiling on a laptop running Linux.
I'm relying on using filesharing to compile directly from a folder. due to permissions and such I don't want to write to this folder
so I need to add this so that I can output the binaries that the build script produces to a local directory.