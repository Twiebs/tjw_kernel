#if 0
internal KFS_Node *
kfs_find_file_with_name(const char *filename, size_t length) {
  for (uint32_t i = 0; i < _fs.kfs->node_count; i++) {
		KFS_Node *node = &_fs.kfs_nodes[i];
		size_t node_filename_length = strlen(node->name);
		if (strings_match(node->name, node_filename_length, filename, length)) {
			return node;
		}
	}
	return 0;
}


internal KFS_Node *
kfs_find_file_with_cstr_name(const char *filename) {
	size_t length = strlen(filename);
  for (uint32_t i = 0; i < _fs.kfs->node_count; i++) {
		KFS_Node *node = &_fs.kfs_nodes[i];
		size_t node_filename_length = strlen(node->name);
		if (strings_match(node->name, node_filename_length, filename, length)) {
			return node;
		}
	}
	return 0;
}

internal uint8_t 
*kfs_get_node_data(const KFS_Node *node) {
	uint8_t *data = _fs.base_data + node->offset;
	return data;
}
#endif