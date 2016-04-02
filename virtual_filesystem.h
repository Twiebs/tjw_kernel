

struct VFS_Node {
	const char name[256];
};


typedef uint32_t (*VFS_Read_Proc)(VFS_Node *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*VFS_Write_Proc)(VFS_Node *, uint32_t, uint32_t, uint8_t *)
typedef void (*VFS_Open_Proc)(VFS_Node *);
typedef void (*VFS_Close_Proc)(VFS_Node *);


