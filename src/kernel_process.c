
#define ELF64_IMPLEMENTATION
#include "elf64.h"

#if 0
static inline
uintptr_t kprocess_get_start_address(uintptr_t elf_executable){
  ELF64Header *header = (ELF64Header *)elf_executable;
  if(header->magicNumber != ELF64_MAGIC_NUMBER){
    klog_error("invalid elf file was provided");
    return 0;
  } 

  return header->programEntryOffset;
}
#endif

static inline
int kprocess_load_elf_executable(uintptr_t elf_executable){
  ELF64Header *header = (ELF64Header *)elf_executable;
  if(header->magicNumber != ELF64_MAGIC_NUMBER){
    klog_error("invalid elf file was provided");
    return 0;
  }
#if 0
  klog_debug("program entry offset: 0x%X", header->programEntryOffset);
  for(size_t i = 0; i < header->programHeaderEntryCount; i++){
    ELF64ProgramHeader *program_header = (ELF64ProgramHeader *)(elf_executable + header->programHeaderOffset + (i * header->programHeaderEntrySize));
    klog_debug("program header entry:");
    klog_debug(" segment virtual memory location: 0x%X", program_header->virtual_address);
    klog_debug(" segment file offset: 0x%X", program_header->offset_in_file);
    klog_debug(" segment file size: 0x%X", program_header->segment_file_size);
    klog_debug(" segment memory size: 0x%X", program_header->segment_memory_size);
  }
  #endif

  return header->programEntryOffset;
}