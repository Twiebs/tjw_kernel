sudo losetup -o 1048576 /dev/loop0 disk.img
sudo mount /dev/loop0 /mnt
sudo cp ../bin/kernel /mnt/boot/kernel
sudo cp ../root/boot/grub.cfg /mnt/boot/grub/grub.cfg
sudo umount /dev/loop0
sudo losetup -d /dev/loop0