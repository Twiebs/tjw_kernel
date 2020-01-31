
typedef enum {
  File_System_Type_INVALID,
  File_System_Type_NONE,
  File_System_Type_EXT2,
} File_System_Type;

typedef enum {
  VFS_Node_Type_INVALID,
  VFS_Node_Type_FILE,
  VFS_Node_Type_DIRECTORY
} VFS_Node_Type;

typedef struct {
  VFS_Node_Type type;
  char name[256];
  size_t name_length;
  uint64_t inode;
} VFS_Node_Info;

typedef struct {
  uint64_t fs_reference;
  uint64_t file_size;
  uint64_t creation_time;
  uint64_t accessed_time;
  uint64_t modified_time; 
} VFS_Node_Handle;


struct Ext2_Filesystem;

typedef struct {
  //TOOD(Torin, 2017-10-23) obviously this must be abstracted!
  void *root;
} Virtual_File_System;

typedef void(*VFS_Node_Info_Procedure)(VFS_Node_Info *, void *);

Error_Code vfs_acquire_node_handle(const char *path, size_t path_length, VFS_Node_Handle *out_handle);
Error_Code vfs_release_node_handle(VFS_Node_Handle *handle);
Error_Code vfs_node_read_file(VFS_Node_Handle *handle, uint64_t offset, uint64_t size, uint8_t *virtual_address);
Error_Code vfs_node_iterate_directory(VFS_Node_Handle *handle, VFS_Node_Info_Procedure procedure, void *userdata);