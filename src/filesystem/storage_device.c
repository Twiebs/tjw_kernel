

Error_Code storage_device_initialize(Storage_Device *storage_device) {
  uint8_t temporary_buffer[512];
  if (storage_device_read(storage_device, 0, 1, temporary_buffer)) {
    klog_error("failed to read storage_device");
    return Error_Code_FAILED_READ;
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

  return Error_Code_NONE;
}

Error_Code storage_device_read(Storage_Device *storage_device, uint64_t block_number, uint64_t block_count, uint8_t *buffer) {
  switch (storage_device->type) {
    case Storage_Device_Type_EHCI: {
      EHCI_Controller *ehci_hc = (EHCI_Controller *)storage_device->controller_ptr;
      USB_Device *usb_device = (USB_Device *)storage_device->device_ptr;
      USB_Mass_Storage_Device *msd = &usb_device->msd;

      //TODO(Torin: 2017-08-10) Make sure block_size == pow2 at init
      //TODO(Torin: 2017-08-11) This is soooooooooo slooooooooowwwwwwwwww
      // but it handles all cases!
      uint64_t blocks_per_page = 4096 / storage_device->block_size;
      uint64_t pages_to_read = block_count / blocks_per_page;
      uint64_t remainder_blocks_to_read = block_count % blocks_per_page;
      uint8_t *temporary_memory = cpu_get_temporary_memory();
      uintptr_t physical_address = memory_get_physical_address((uintptr_t)temporary_memory);
      uint64_t current_block_number = block_number;
      uint8_t *write_ptr = buffer;

      //TODO(Torin: 2017-08-10) Make ehci_read_to_physical_address handle multiple block reads internaly
      for (size_t i = 0; i < pages_to_read; i++) {
        for (size_t j = 0; j < blocks_per_page; j++) {
          ehci_read_to_physical_address(ehci_hc, msd, physical_address + (j * storage_device->block_size),current_block_number + j, 1);
        }
        memory_copy(write_ptr, temporary_memory, 4096);
        write_ptr += 4096;
        current_block_number += blocks_per_page;
      }

      for (size_t i = 0; i < remainder_blocks_to_read; i++) {
        ehci_read_to_physical_address(ehci_hc, msd, physical_address + (i * storage_device->block_size), current_block_number + i, 1);
      }

      memory_copy(write_ptr, temporary_memory, remainder_blocks_to_read * storage_device->block_size);
      return Error_Code_NONE;
    } break;

    case Storage_Device_Type_INVALID:{
      klog_error("invalid storage device");
      return Error_Code_INVALID_DATA;
    } break;

    default:{
      klog_error("usupported storage device");
      return Error_Code_UNSUPORTED_FEATURE;
    } break;
  }

  return Error_Code_NONE;
}