qemu-system-i386 -s -S -kernel bin/kernel &
gdb --eval-command="target remote localhost:1234" --eval-command="symbol-file bin/debug_symbols"

