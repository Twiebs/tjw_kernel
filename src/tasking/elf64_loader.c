
#if 0
bool extract_elf_executable_load_info(uintptr_t executable_virtual, Executable_Load_Info *info){
  
  info->entry_address = elf_header->entry_address;

  ELF64_Program_Header *program_header = (ELF64_Program_Header *)(executable_virtual + elf_header->program_header_offset);
  for(size_t i = 0; i < elf_header->program_header_entry_count; i++){
    if(program_header->segment_type == ELF_Segment_Type_LOAD){
      if(info->code_location != 0x00){
        klog_error("unhandled multipule load case");
        return false;
      }

      info->code_location = program_header->virtual_address;
      info->code_offset = program_header->offset_in_file;
    }

    program_header = (ELF64_Program_Header *)(((uintptr_t)program_header) + elf_header->program_header_entry_size);
  }

  return true;
}
#endif

bool elf64_is_header_valid(ELF64_Header *elf64_header) {
  if (elf64_header->magic_number != ELF64_MAGIC_NUMBER) {
    klog_error("program binary does not have a valid elf64 signature");
    return false;
  }

  if (elf64_header->elf_class != ELF_CLASS_64) {
    klog_error("ELF executable must be 64bit format");
    return false;
  }

  if (elf64_header->data_encoding != ELF_DATA_ENCODING_LITTLE_ENDIAN) {
    klog_error("ELF Executable must be endcoded Little Endian");
    return false;
  }

  if(elf64_header->abi_type != ELF_ABI_TYPE_SYSTEMV) {
    klog_error("ELF executable must use SystemV ABI");
    return false;
  }

  return true;
}


Error_Code process_create_from_elf64_file_string(const char *path, size_t length) {
  VFS_Node_Handle handle = {};
  if (vfs_acquire_node_handle(path, length, &handle)) {
    klog_error("could not open elf executable, file not found");
    return Error_Code_VFS_NODE_NOT_FOUND;
  }

  ELF64_Header header = {};
  Buffered_File_Reader reader = {};
  buffered_file_reader_initialize(&reader, &handle);
  buffered_file_reader_read_bytes_to_buffer(&reader, &header, sizeof(ELF64_Header));

  if (elf64_is_header_valid(&header) == false) {
    return Error_Code_INVALID_DATA;
  }

  klog_debug("read elf");
  return Error_Code_NONE;
}