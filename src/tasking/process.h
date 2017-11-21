
#include <stdint.h>

typedef struct {
  uintptr_t entry_address;
  uintptr_t code_virtual_address;
  uint64_t code_size;
  uintptr_t data_location;
  uint64_t data_offset;
  uint64_t data_size; 
} Process_Create_Info;

typedef struct {
  uint64_t process_id;
} Process_ID;

typedef struct {
  Virtual_Address p4_page_table;
  Virtual_Address code_address;
  uint64_t code_page_count;
  Virtual_Address data_address;
  uint64_t data_page_count;
  //threads;
  //Allocated memory
  //vfs handles
} Process;

Error_Code process_create(Process_Create_Info *create_info, Process_ID *result_id);