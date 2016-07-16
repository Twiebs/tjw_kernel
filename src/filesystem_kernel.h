#define KFS_HEADER_VERIFIER (('k' << 24) | ('f' << 16) | ('s' << 8) | ('h'))
#define KFS_NODE_VERIFIER (('k' << 24) | ('f' << 16) | ('s' << 8) | ('n'))

typedef struct {
	uint32_t verifier;
	uint32_t node_count;
} KFS_Header;

typedef struct {
	uint32_t offset;
	uint32_t size;
	const char name[256];
} KFS_Node;

internal KFS_Node *
kfs_find_file_with_name(const char *filename, size_t length);

internal KFS_Node *
kfs_find_file_with_cstr_name(const char *filename);

internal uint8_t 
*kfs_get_node_data(const KFS_Node *node);

