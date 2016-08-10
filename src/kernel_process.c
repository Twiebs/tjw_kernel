
#define ELF64_IMPLEMENTATION
#include "elf64.h"

static inline
int kprocess_load_elf_executable(uintptr_t elf_executable){
  ELF64Header *header = (ELF64Header *)elf_executable;
  if(header->magicNumber != ELF64_MAGIC_NUMBER){
    klog_error("invalid elf file was provided");
    return 0;
  } else {
    klog_info("elf file was valid!!!");
  }

  ELF64ProgramHeader *program_header = (ELF64ProgramHeader *)(elf_executable + header->programHeaderOffset);
  klog_debug("program entry address: %lu", header->programEntryOffset);
  klog_debug("program_header virtual_address: %lu", program_header->virtual_address);

#if 0
  ELFSectionHeader *sectionStringTableSectionHeader = (ELFSectionHeader*)(elf_executable + header->sectionHeaderOffset + ((header->sectionStringTableSectionIndex) * header->sectionHeaderEntrySize));
  char *sectionStringTableData = (char *)(elf_executable + (sectionStringTableSectionHeader->fileOffsetOfSectionData));
  kassert(*sectionStringTableData == 0); //First byte of the stringTableData is always null


  for(size_t i = 0; i < header->sectionHeaderEntryCount; i++){
    ELFSectionHeader *section = (ELFSectionHeader *)(elf_executable + header->sectionHeaderOffset + ((i+1) * header->sectionHeaderEntrySize));
    const char *section_name = (const char *)(elf_executable + section->nameOffset);
    klog_debug("found elf section: %s", section_name);
    section = (ELFSectionHeader *)((uintptr_t)section + section->sectionSize);
  }
  #endif

  return 1;
}