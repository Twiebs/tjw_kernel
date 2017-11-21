#include <stdint.h>

static const uint32_t ELF64_MAGIC_NUMBER = (('F' << 24) | ('L' << 16) | ('E' << 8) | 0x7F);


#define ELF_OBJ_TYPE_NONE 0
#define ELF_OBJ_TYPE_RELOCATABLE 1
#define ELF_OBJ_TYPE_EXECUTABLE 2
#define ELF_OBJ_TYPE_DYNAMIC 3
#define ELF_OBJ_TYPE_CORE 4

#define ELF_SECTION_TYPE_INVALID 0
#define ELF_SECTION_TYPE_PROGRAM_INFO 1
#define ELF_SECTION_TYPE_SYMBOL_TABLE 2
#define ELF_SECTION_TYPE_STRING_TABLE 3
#define ELF_SECTION_TYPE_UNINIALIZED_SPACE 8

#define ELF_SECTION_FLAG_WRITE 1
#define ELF_SECTION_FLAG_STATIC_MEMORY 2
#define ELF_SECTION_FLAG_EXECUTABLE 4

#define ELF_SYMBOL_BINDING_LOCAL  0
#define ELF_SYMBOL_BINDING_GLOBAL 1
#define ELF_SYMBOL_BINDING_WEAK   2

#define ELF_SYMBOL_TYPE_NOTYPE 0
#define ELF_SYMBOL_TYPE_OBJECT 1
#define ELF_SYMBOL_TYPE_FUNCTION 2
#define ELF_SYMBOL_TYPE_SECTION 3
#define ELF_SYMBOL_TYPE_FILE 4

static const uint8_t ELF_CLASS_32 = 1;
static const uint8_t ELF_CLASS_64 = 2;
static const uint8_t ELF_DATA_ENCODING_LITTLE_ENDIAN = 1;
static const uint8_t ELF_DATA_ENCODING_BIG_ENDIAN = 2;
static const uint8_t ELF_ABI_TYPE_SYSTEMV = 0;
static const uint8_t ELF_ABI_TYPE_HPUX = 1;
static const uint8_t ELF_ABI_TYPE_STANDALONE = 255; 

typedef struct {
  uint32_t magic_number; //0-3
  uint8_t elf_class;     //4
  uint8_t data_encoding; //5
  uint8_t elf_version;   //6
  uint8_t abi_type;      //7
  uint8_t abi_version;   //8
  uint8_t padding[7];    //9-15

  uint16_t object_type;
  uint16_t machine_type;
  uint32_t object_version;
  
  uint64_t entry_address; 
  uint64_t program_header_offset;
  uint64_t section_header_offset;
  uint32_t flags;

  uint16_t elf_header_size;
  uint16_t program_header_entry_size;
  uint16_t program_header_entry_count;
  uint16_t section_header_entry_size;
  uint16_t section_header_entry_count;
  uint16_t section_name_string_table_index;
} __attribute((packed)) ELF64_Header;

typedef struct {
  uint32_t segment_type;
  uint32_t flags;
  uint64_t offset_in_file; 
  uint64_t virtual_address;
  uint64_t undefined;
  uint64_t segment_file_size;
  uint64_t segment_memory_size;
  uint64_t alignment;
} __attribute((packed)) ELF64_Program_Header;

typedef struct {
  uint32_t name_offset;
  uint32_t section_type;
  uint64_t flags;
  uint64_t virtualAddress;
  uint64_t fileOffsetOfSectionData;
  uint64_t sectionSize;

  uint32_t sectionLink;
  uint32_t sectionInfo;
  uint64_t addressAlignmentBoundary;
  uint64_t sectionEntrySize;
} ELF_Section_Header;

typedef struct {
  uint32_t nameOffset;
  uint8_t type    : 4;
  uint8_t binding : 4;
  uint8_t nullByte;
  uint16_t sectionTableIndex;
  uint64_t symbolValue;
  uint64_t size;
} ELFSymbol;

typedef enum {
  ELF_Segment_Type_NULL = 0,
  ELF_Segment_Type_LOAD = 1,
  ELF_Segment_Type_DYNAMIC = 2,
  ELF_Segment_Type_INTERP = 3,
  ELF_Segment_Type_NOTE = 4,
  ELF_Segment_Type_SHLIB = 5,
  ELF_Segment_Type_PHDR = 6,
  ELF_Segment_Type_LOPROC = 0x70000000,
  ELF_Segment_Type_HIPROC = 0x7FFFFFFF 
} ELF_Segment_Type;

typedef enum {
  ELF_SECTION_TEXT,
  ELF_SECTION_BSS,
  ELF_SECTION_DATA,
} ELFSection;
