sh scripts/upload.sh
sudo qemu-custom -s -serial file:serial.log -D qemu.log -d int,pcall -hdb /dev/sde -no-reboot -m 4G &
gdb --eval-command="set arch i386:x86-64" --eval-command="target remote localhost:1234" --eval-command="symbol-file bin/debug_symbols"
