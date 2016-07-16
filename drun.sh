sh upload.sh
sudo qemu-custom -s -serial file:serial.log -D qemu.log -d int,pcall -hdb /dev/sde -no-reboot -m 4G &
sh gdb_connect.sh
