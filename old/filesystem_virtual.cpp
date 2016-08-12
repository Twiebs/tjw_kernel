
#if 0	
	uint32_t *module_addr_array = (uint32_t *)((uintptr_t)mb->mods_addr);
	KFS_Header *kfs = (KFS_Header *)(uintptr_t)module_addr_array[0];
	bool is_kfs_valid = (kfs->verifier == KFS_HEADER_VERIFIER);
	klog("KFS Status: %s", is_kfs_valid ? "VALID" : "INVALID");
	if (is_kfs_valid) {
		klog("KFS File Count: %u", kfs->node_count);
	}

	_fs.kfs = kfs;
	_fs.kfs_nodes = (KFS_Node *)(kfs + 1);
	_fs.base_data = (uint8_t *)(_fs.kfs_nodes + kfs->node_count);
#endif

