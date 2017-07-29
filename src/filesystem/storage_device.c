

int storage_device_initialize(Storage_Device *storage_device) {
  uint8_t temporary_buffer[512];
  if (storage_device_read_to_physical(storage_device, 0, 1, (uintptr_t)temporary_buffer)) {
    klog_error("failed to read storage_device");
    return 1;
  }

  MBR_Partition_Table *partition_table = (MBR_Partition_Table *)(temporary_buffer + 0x1BE);
  for (size_t partition_index = 0; partition_index < 4; partition_index++) {
    //NOTE(Torin) If a MBR partition is unused all fields will be zero.
    //The partition table is 16 bytes so we do two 8byte compares
    uint64_t *raw_data = (uint64_t *)partition_table;
    if (raw_data[0] == 0 && raw_data[1] == 0) continue;

    kassert(storage_device->partition_count < 4);
    Storage_Device_Partition *partition = &storage_device->partitions[storage_device->partition_count];
    uint64_t current_partition_index = storage_device->partition_count++;
    partition->file_system_type = File_System_Type_NONE;
    partition->first_block = partition_table->start_sector;
    partition->block_count = partition_table->sector_count;

    //Ext2 Filesystem
    if (partition_table->system_id == MBR_Partition_Type_LINUX_FILESYSTEM) {
      //TODO(Torin: 2017-07-26) Where should this come from?
      partition->file_system_type = File_System_Type_EXT2;
      Ext2_Filesystem *extfs = &globals.ext2_filesystem;
      ext2_file_system_initalize(extfs, storage_device, current_partition_index);
    }
    partition_table++;
  }
  return 0;
}

int storage_device_read_to_physical(Storage_Device *storage_device, uint64_t block_number, uint64_t block_count, uintptr_t physical_address) {
  switch (storage_device->type) {
    case Storage_Device_Type_EHCI: {
      EHCI_Controller *ehci_hc = (EHCI_Controller *)storage_device->controller_ptr;
      USB_Device *usb_device = (USB_Device *)storage_device->device_ptr;
      USB_Mass_Storage_Device *msd = &usb_device->msd;
      if (ehci_read_to_physical_address(ehci_hc, msd, physical_address, block_number, block_count)) {
        klog_error("[Storage_Device] Read failed");
      }

    } break;

    case Storage_Device_Type_INVALID:{
      klog_error("invalid storage device");
      return 1;
    } break;

    default:{
      klog_error("usupported storage device");
      return 1;
    } break;
  }

  return 0;
}