
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

typedef enum {
  VFS_Node_Type_INVALID,
  VFS_Node_Type_FILE,
  VFS_Node_Type_DIRECTORY
} VFS_Node_Type;

typedef struct {
  char name[256];
  VFS_Node_Type type;
  uint64_t inode;
} VFS_Node;

typedef struct {
  uint64_t fs_reference;
  uint64_t file_size;
  uint64_t creation_time;
  uint64_t accessed_time;
  uint64_t modified_time; 
} VFS_Node_Handle;

VFS_Node_Handle vfs_aquire_node_handle(const char *path);
Error_Code vfs_release_node_handle(VFS_Node_Handle handle);
Error_Code vfs_node_read(VFS_Node_Handle handle, uint64_t offset, uint64_t size);

typedef struct {

} Virtual_File_System;

int fs_read_file(File_Handle *h, uint64_t offset, uint64_t size, uintptr_t *physical_pages);
int fs_obtain_file_handle(const char *path, uint64_t path_length, File_Handle *in_handle);