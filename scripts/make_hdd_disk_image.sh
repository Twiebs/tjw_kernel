#!/bin/bash
source $(dirname $0)/variables.sh

let BLOCK_SIZE=512
let DISK_SPACE_SIZE_IN_MB=32
let BLOCK_COUNT=($DISK_SPACE_SIZE_IN_MB*1024*1024)/$BLOCK_SIZE

sudo dd if=/dev/zero of=$IMAGE_OUTPUT_DIRECTORY/hdd.img bs=$BLOCK_SIZE count=$BLOCK_COUNT
#sudo losetup /dev/loop0 $IMAGE_OUTPUT_DIRECTORY/hdd.img
#mkfs -t ext2 /dev/loop0
#mount -t ext2 /dev/loop0 /mnt
#umount /dev/loop0