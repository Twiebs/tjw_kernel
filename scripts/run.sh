FLASH_DRIVE_DEVICE=/dev/sdb
FLASH_DRIVE_PARTITION=/dev/sdb1
KERNEL_BINARY_DIRECTORY=~/output
WORKING_DIRECTORY=~/output

mkdir $WORKING_DIRECTORY/kernel_mount_temp
sudo umount $FLASH_DRIVE_PARTITION
sudo mount $FLASH_DRIVE_PARTITION $WORKING_DIRECTORY/kernel_mount_temp
sudo cp $KERNEL_BINARY_DIRECTORY/kernel $WORKING_DIRECTORY/kernel_mount_temp/boot/kernel
sleep 1
sudo umount $FLASH_DRIVE_PARTITION
rmdir kernel_mount_temp
sudo qemu-system-x86_64 -hda $FLASH_DRIVE_DEVICE \
  -smp 1 -m 512 \
  -device usb-ehci,id=ehci \
  -serial file:$WORKING_DIRECTORY/serial.out -s