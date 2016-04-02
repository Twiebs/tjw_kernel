
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define TJW_PLATFORM_IMPLEMENTATION
#include "../../tjw/tjw_platform.h"
#include "../kernel_filesystem.h"

int main(int argc, char **argv) {
	static const char *search_directory = "../kfs/";

	KFS_Header header = {};
	header.verifier = KFS_HEADER_VERIFIER;
	header.node_count = get_file_count_in_directory(search_directory);

	KFS_Node *nodes = (KFS_Node *)malloc(sizeof(KFS_Node) * header.node_count);
	uint8_t **data = (uint8_t **)malloc(sizeof(uint8_t *) * header.node_count);

	size_t current_node_index = 0;
	size_t current_offset = 0;
	foreach_file_in_directory("../kfs/", [&](FileInfo info) {
		KFS_Node *node = &nodes[current_node_index];
		memcpy((void*)node->name, info.name, 256);
		FILE *file = fopen(info.filepath, "rb");
		if (file) {
			fseek(file, 0, SEEK_END);
			size_t file_size = ftell(file);
			fseek(file, 0, SEEK_SET);
			data[current_node_index] = (uint8_t *)malloc(file_size);
			fread(data[current_node_index], file_size, 1, file);
			fclose(file);
			node->size = file_size;
			node->offset = current_offset;
			current_offset += file_size;
		} else {
			assert(false);
		}
		current_node_index++;
	});

	FILE *kfs_file = fopen("../bin/test.kfs", "wb");
	fwrite(&header, sizeof(header), 1, kfs_file);
	fwrite(nodes, sizeof(KFS_Node) * header.node_count, 1, kfs_file);
	for (uint32_t i = 0; i < header.node_count; i++) {
		fwrite(data[i], nodes[i].size, 1, kfs_file);
	}
	fclose(kfs_file);

	printf("%u files written\n", header.node_count);

	
	uint8_t *data = read_file_into_memory_and_null_terminate("../bin/test.kfs");
	if (data) {
		KFS_Header *header = (KFS_Header *)data;
		KFS_Node *nodes = (KFS_Node *)(header + 1);
		assert(header->verifier == KFS_HEADER_VERIFIER);
		for (uint32_t i = 0; 

	} else {
		assert(false);
	}

}
