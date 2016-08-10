#ifndef ELF64_HEADER_GUARD
#define ELF64_HEADER_GUARD

#ifdef ELF64_IMPLEMENTATION
#undef ELF64_IMPLEMENTATION

#include <stdint.h>

#define ELF64_MAGIC_NUMBER (('F' << 24) | ('L' << 16) | ('E' << 8) | 0x7F)

#define ELF_DATA_ENCODING_LITTLE_ENDIAN 1
#define ELF_DATA_ENCODING_BIG_ENDIAN 2

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

typedef struct {
  uint32_t magicNumber;
  uint8_t bitType;
  uint8_t dataEncoding;
  uint8_t elfVersion;
  uint8_t osABI;
  uint8_t padding[8];

  uint16_t objectFileType;
  uint16_t instructionSet;
  uint32_t elfVersion11;

  uint64_t programEntryOffset;
  uint64_t programHeaderOffset;
  uint64_t sectionHeaderOffset;

  uint32_t flags;
  uint16_t elfHeaderSize;
  uint16_t programHeaderEntrySize;
  uint16_t programHeaderEntryCount;
  uint16_t sectionHeaderEntrySize;
  uint16_t sectionHeaderEntryCount;
  uint16_t sectionStringTableSectionIndex;
} ELF64Header;

typedef struct {
  uint32_t nameOffset;
  uint32_t sectionType;
  uint64_t flags;
  uint64_t virtualAddress;
  uint64_t fileOffsetOfSectionData;
  uint64_t sectionSize;

  uint32_t sectionLink;
  uint32_t sectionInfo;
  uint64_t addressAlignmentBoundary;
  uint64_t sectionEntrySize;
} ELFSectionHeader;

typedef struct {
  uint32_t segment_type;
  uint32_t flags;
  uint64_t offset; 
  uint64_t virtual_address;
  uint64_t undefined;
  uint64_t segment_file_size;
  uint64_t segment_memory_size;
  uint64_t alignment;
} ELF64ProgramHeader;

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
  ELF_SECTION_TEXT,
  ELF_SECTION_BSS,
  ELF_SECTION_DATA,
} ELFSection;


#define DW_TAG_META_LIST                                                       \
  _(DW_TAG_array_type, 0x01)                                                   \
  _(DW_TAG_class_type, 0x02)                                                   \
  _(DW_TAG_entry_point, 0x03)                                                  \
  _(DW_TAG_enumeration_type, 0x04)                                             \
  _(DW_TAG_formal_parameter, 0x05)                                             \
  _(DW_TAG_imported_declaration, 0x08)                                         \
  _(DW_TAG_label, 0x0a)                                                        \
  _(DW_TAG_lexical_block, 0x0b)                                                \
  _(DW_TAG_member, 0x0d)                                                       \
  _(DW_TAG_pointer_type, 0x0f)                                                 \
  _(DW_TAG_reference_type, 0x10)                                               \
  _(DW_TAG_compile_unit, 0x11)                                                 \
  _(DW_TAG_string_type, 0x12)                                                  \
  _(DW_TAG_structure_type, 0x13)                                               \
  _(DW_TAG_subroutine_type, 0x15)                                              \
  _(DW_TAG_typedef, 0x16)                                                      \
  _(DW_TAG_union_type, 0x17)                                                   \
  _(DW_TAG_unspecified_parameters, 0x18)                                       \
  _(DW_TAG_variant, 0x19)                                                      \
  _(DW_TAG_common_block, 0x1a)                                                 \
  _(DW_TAG_common_inclusion, 0x1b)                                             \
  _(DW_TAG_inheritance, 0x1c)                                                  \
  _(DW_TAG_inlined_subroutine, 0x1d)                                           \
  _(DW_TAG_module, 0x1e)                                                       \
  _(DW_TAG_ptr_to_member_type, 0x1f)                                           \
  _(DW_TAG_set_type, 0x20)                                                     \
  _(DW_TAG_subrange_type, 0x21)                                                \
  _(DW_TAG_with_stmt, 0x22)                                                    \
  _(DW_TAG_access_declaration, 0x23)                                           \
  _(DW_TAG_base_type, 0x24)                                                    \
  _(DW_TAG_catch_block, 0x25)                                                  \
  _(DW_TAG_const_type, 0x26)                                                   \
  _(DW_TAG_constant, 0x27)                                                     \
  _(DW_TAG_enumerator, 0x28)                                                   \
  _(DW_TAG_file_type, 0x29)                                                    \
  _(DW_TAG_friend, 0x2a)                                                       \
  _(DW_TAG_namelist, 0x2b)                                                     \
  _(DW_TAG_namelist_item, 0x2c)                                                \
  _(DW_TAG_packed_type, 0x2d)                                                  \
  _(DW_TAG_subprogram, 0x2e)                                                   \
  _(DW_TAG_template_type_parameter, 0x2f)                                      \
  _(DW_TAG_template_value_parameter, 0x30)                                     \
  _(DW_TAG_thrown_type, 0x31)                                                  \
  _(DW_TAG_try_block, 0x32)                                                    \
  _(DW_TAG_variant_part, 0x33)                                                 \
  _(DW_TAG_variable, 0x34)                                                     \
  _(DW_TAG_volatile_type, 0x35)                                                \
  _(DW_TAG_dwarf_procedure, 0x36)                                              \
  _(DW_TAG_restrict_type, 0x37)                                                \
  _(DW_TAG_interface_type, 0x38)                                               \
  _(DW_TAG_namespace, 0x39)                                                    \
  _(DW_TAG_imported_module, 0x3a)                                              \
  _(DW_TAG_unspecified_type, 0x3b)                                             \
  _(DW_TAG_partial_unit, 0x3c)                                                 \
  _(DW_TAG_imported_unit, 0x3d)                                                \
  _(DW_TAG_condition, 0x3f)                                                    \
  _(DW_TAG_shared_type, 0x40)                                                  \
  _(DW_TAG_type_unit, 0x41)                                                    \
  _(DW_TAG_rvalue_reference_type, 0x42)                                        \
  _(DW_TAG_template_alias, 0x43)                                               \
  _(DW_TAG_lo_user, 0x4080) _(DW_TAG_hi_user, 0xffff)

#define DW_AT_META_LIST \
  _(DW_AT_sibling, 0x01) \
  _(DW_AT_location, 0x02) \
  _(DW_AT_name, 0x03) \
  _(DW_AT_ordering, 0x09) \
  _(DW_AT_byte_size, 0x0b) \
  _(DW_AT_bit_offset, 0x0c) \
  _(DW_AT_bit_size, 0x0d) \
  _(DW_AT_stmt_list, 0x10) \
  _(DW_AT_low_pc, 0x11) \
  _(DW_AT_high_pc, 0x12) \
  _(DW_AT_language, 0x13) \
  _(DW_AT_discr, 0x15) \
  _(DW_AT_discr_value, 0x16) \
  _(DW_AT_visibility, 0x17) \
  _(DW_AT_import, 0x18) \
  _(DW_AT_string_length, 0x19) \
  _(DW_AT_common_reference, 0x1a) \
  _(DW_AT_comp_dir, 0x1b) \
  _(DW_AT_const_value, 0x1c) \
  _(DW_AT_containing_type, 0x1d) \
  _(DW_AT_default_value, 0x1e) \
  _(DW_AT_inline, 0x20) \
  _(DW_AT_is_optional, 0x21) \
  _(DW_AT_lower_bound, 0x22) \
  _(DW_AT_producer, 0x25) \
  _(DW_AT_prototyped, 0x27) \
  _(DW_AT_return_addr, 0x2a) \
  _(DW_AT_start_scope, 0x2c) \
  _(DW_AT_bit_stride, 0x2e) \
  _(DW_AT_upper_bound, 0x2f) \
  _(DW_AT_abstract_origin, 0x31) \
  _(DW_AT_accessibility, 0x32) \
  _(DW_AT_address_class, 0x33) \
  _(DW_AT_artificial, 0x34) \
  _(DW_AT_base_types, 0x35) \
  _(DW_AT_calling_convention, 0x36) \
  _(DW_AT_count, 0x37) \
  _(DW_AT_data_member_location, 0x38) \
  _(DW_AT_decl_column, 0x39) \
  _(DW_AT_decl_file, 0x3a) \
  _(DW_AT_decl_line, 0x3b) \
  _(DW_AT_declaration, 0x3c) \
  _(DW_AT_discr_list, 0x3d) \
  _(DW_AT_encoding, 0x3e) \
  _(DW_AT_external, 0x3f) \
  _(DW_AT_frame_base, 0x40) \
  _(DW_AT_friend, 0x41) \
  _(DW_AT_identifier_case, 0x42) \
  _(DW_AT_macro_info, 0x43) \
  _(DW_AT_namelist_item, 0x44) \
  _(DW_AT_priority, 0x45) \
  _(DW_AT_segment, 0x46) \
  _(DW_AT_specification, 0x47) \
  _(DW_AT_static_link, 0x48) \
  _(DW_AT_type, 0x49) \
  _(DW_AT_use_location, 0x4a) \
  _(DW_AT_variable_parameter, 0x4b) \
  _(DW_AT_virtuality, 0x4c) \
  _(DW_AT_vtable_elem_location, 0x4d) \
  _(DW_AT_allocated, 0x4e) \
  _(DW_AT_associated, 0x4f) \
  _(DW_AT_data_location, 0x50) \
  _(DW_AT_byte_stride, 0x51) \
  _(DW_AT_entry_pc, 0x52) \
  _(DW_AT_use_UTF8, 0x53) \
  _(DW_AT_extension, 0x54) \
  _(DW_AT_ranges, 0x55) \
  _(DW_AT_trampoline, 0x56) \
  _(DW_AT_call_column, 0x57) \
  _(DW_AT_call_file, 0x58) \
  _(DW_AT_call_line, 0x59) \
  _(DW_AT_description, 0x5a) \
  _(DW_AT_binary_scale, 0x5b) \
  _(DW_AT_decimal_scale, 0x5c) \
  _(DW_AT_small, 0x5d) \
  _(DW_AT_decimal_sign, 0x5e) \
  _(DW_AT_digit_count, 0x5f) \
  _(DW_AT_picture_string, 0x60) \
  _(DW_AT_mutable, 0x61) \
  _(DW_AT_threads_scaled, 0x62) \
  _(DW_AT_explicit, 0x63) \
  _(DW_AT_object_pointer, 0x64) \
  _(DW_AT_endianity, 0x65) \
  _(DW_AT_elemental, 0x66) \
  _(DW_AT_pure, 0x67) \
  _(DW_AT_recursive, 0x68) \
  _(DW_AT_signature, 0x69) \
  _(DW_AT_main_subprogram, 0x6a) \
  _(DW_AT_data_bit_offset, 0x6b) \
  _(DW_AT_const_expr, 0x6c) \
  _(DW_AT_enum_class, 0x6d) \
  _(DW_AT_linkage_name, 0x6e) \
  _(DW_AT_lo_user, 0x2000) \
  _(DW_AT_hi_user, 0x3fff) \

#define DW_FORM_META_LIST \
_(DW_FORM_addr, 0x01) \
_(DW_FORM_block2, 0x03) \
_(DW_FORM_block4, 0x04) \
_(DW_FORM_data2, 0x05) \
_(DW_FORM_data4, 0x06) \
_(DW_FORM_data8, 0x07) \
_(DW_FORM_string, 0x08) \
_(DW_FORM_block, 0x09) \
_(DW_FORM_block1, 0x0a) \
_(DW_FORM_data1, 0x0b) \
_(DW_FORM_flag, 0x0c) \
_(DW_FORM_sdata, 0x0d) \
_(DW_FORM_strp, 0x0e) \
_(DW_FORM_udata, 0x0f) \
_(DW_FORM_ref_addr, 0x10) \
_(DW_FORM_ref1, 0x11) \
_(DW_FORM_ref2, 0x12) \
_(DW_FORM_ref4, 0x13) \
_(DW_FORM_ref8, 0x14) \
_(DW_FORM_ref_udata, 0x15) \
_(DW_FORM_indirect, 0x16) \
_(DW_FORM_sec_offset, 0x17 ) \
_(DW_FORM_exprloc, 0x18) \
_(DW_FORM_flag_present, 0x19) \
_(DW_FORM_ref_sig8, 0x20) \

#define _(name,value) static const uint32_t name = value;
  DW_AT_META_LIST
  DW_TAG_META_LIST
  DW_FORM_META_LIST
#undef _

#define _(attrib_name, attrib_value) \
else if (attrib_value == value) { \
  return #attrib_name; \
}

const char *dw_get_tag_string(uint32_t value) {
  if (0) {} DW_TAG_META_LIST
  else { return "INVALID!!!"; }
}

const char *dw_get_attrib_string(uint32_t value) {
  if (0) {} DW_AT_META_LIST
  else { return "INVALID!!!"; }
}

const char *dw_get_form_string(uint32_t value) {
  if (0) {} DW_FORM_META_LIST
  else { return "INVALID!!!"; }
}

#undef _

#define DW_CHILDREN_no  0
#define DW_CHILDREN_yes 1

#endif//ELF64_IMPLEMENTATION
#endif//ELF64_HEADER_GUARD
