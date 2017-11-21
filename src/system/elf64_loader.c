
bool extract_elf_executable_load_info(uintptr_t executable_virtual, Executable_Load_Info *info){
  ELF64_Header *elf_header = (ELF64_Header *)executable_virtual;
  exit_if(elf_header->magic_number != ELF64_MAGIC_NUMBER, "ELF executable magic number mismatch");
  exit_if(elf_header->elf_class != ELF_CLASS_64, "ELF executable must be 64bit format");
  exit_if(elf_header->data_encoding != ELF_DATA_ENCODING_LITTLE_ENDIAN, "ELF Executable must be endcoded Little Endian");
  exit_if(elf_header->abi_type != ELF_ABI_TYPE_SYSTEMV, "ELF executable must use SystemV ABI");
  
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



void create_process_from_elf64_file(String_Span path) {
  

}