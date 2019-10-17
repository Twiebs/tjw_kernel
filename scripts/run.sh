mkdir kernel_mount_temp
sudo umount /dev/sdd1
sudo mount /dev/sdd1 kernel_mount_temp
sudo cp /home/torin/workspace/tjw_kernel/bin/kernel kernel_mount_temp/boot/kernel
sleep 1
sudo umount /dev/sdd1
rmdir kernel_mount_temp
sudo qemu-system-x86_64 -hda /dev/sdd \
  -smp 8 -m 4096 \
  -drive if=none,id=goat,file=filesystem.img \
  -device usb-ehci,id=ehci \
  -device usb-storage,bus=ehci.0,drive=goat \
  -serial file:serial.out -s