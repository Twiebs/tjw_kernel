sh upload.sh
sudo qemu-custom -serial file:serial.log -hdb /dev/sde -m 4G -s -S &
sh gdb_connect.sh