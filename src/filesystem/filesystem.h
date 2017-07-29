
typedef enum {
  File_System_Type_INVALID,
  File_System_Type_NONE,
  File_System_Type_EXT2,
} File_System_Type;

typedef struct {
  uint64_t fs_reference;
  uint64_t file_size;
  uint64_t creation_time;
  uint64_t accessed_time;
  uint64_t modified_time; 
} File_Handle;

typedef struct {

} Virtual_File_System;

int fs_read_file(File_Handle *h, uint64_t offset, uint64_t size, uintptr_t *physical_pages);
int fs_obtain_file_handle(const char *path, uint64_t path_length, File_Handle *in_handle);