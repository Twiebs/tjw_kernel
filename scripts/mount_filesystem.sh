sudo losetup --offset 1048576 /dev/loop0 filesystem.img
sudo mount /dev/loop0 filesystem_mount