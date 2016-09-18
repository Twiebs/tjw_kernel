sh upload.sh
sudo qemu-custom -serial file:serial.log -hdb /dev/sde -usb -device usb-ehci,id=ehic -m 4G -smp 8 