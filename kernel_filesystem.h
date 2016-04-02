#define KFS_HEADER_VERIFIER (('k' << 24) | ('f' << 16) | ('s' << 8) | ('h'))
#define KFS_NODE_VERIFIER (('k' << 24) | ('f' << 16) | ('s' << 8) | ('n'))

struct KFS_Header {
	uint32_t verifier;
	uint32_t node_count;
};

struct KFS_Node {
	uint32_t offset;
	uint32_t size;
	const char name[256];
};


