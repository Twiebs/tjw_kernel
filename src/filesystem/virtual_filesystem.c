
Error_Code vfs_node_read_file(VFS_Node_Handle *handle, uint64_t offset, uint64_t size, uint8_t *virtual_address) {
  if ((offset + size) > handle->file_size) return Error_Code_INVALID_DATA;
  if (virtual_address == 0) return Error_Code_INVALID_DATA;

  Ext2_Inode inode = {}; 
  Ext2_Filesystem *ext2fs = &globals.ext2_filesystem;
  if (ext2_inode_read(ext2fs, handle->fs_reference, &inode)) {
    klog_error("failed to read file inode");
    return Error_Code_FAILED_READ;
  }

  if (inode.type != EXT2_INODE_TYPE_REGULAR_FILE) {
    klog_error("[VFS] Inode %u is not a file", handle->fs_reference);
    ext2_debug_log_inode(&inode);
    return Error_Code_INVALID_DATA;
  }

  if (ext2_inode_read_data(ext2fs, &inode, offset, size, virtual_address))
    return Error_Code_FAILED_READ;

  return Error_Code_NONE;
}

Error_Code vfs_aquire_node_handle(const char *path, size_t path_length, VFS_Node_Handle *out_handle) {
  kassert(path_length != 0);
  kassert(path != 0);

  Ext2_Filesystem *extfs = &globals.ext2_filesystem;
  if (path[0] != '/') {
    klog_error("malformed path.  Path's must begin with a '/'");
    return Error_Code_VFS_NODE_NOT_FOUND;
  }

  uint32_t current_inode_number = 2;
  uint8_t buffer[4096] = {};
  Ext2_Inode current_inode = {};


  if (path_length == 1) {
    if (ext2_inode_read(extfs, current_inode_number, &current_inode)) {
      klog_error("failed root inode read");
      return Error_Code_FAILED_READ;
    }

    out_handle->fs_reference = current_inode_number;
    out_handle->file_size = (uint64_t)current_inode.size_low | ((uint64_t)current_inode.size_high << 32LL);
    out_handle->creation_time = current_inode.creation_time;
    out_handle->accessed_time = current_inode.last_access_time;
    out_handle->modified_time = current_inode.last_modification_time;
    return Error_Code_NONE;
  }

  size_t index = 0;
  while (index < path_length) {
    if (path[index] != '/') {
      klog_error("malformed path");
      return Error_Code_VFS_NODE_NOT_FOUND;
    }

    index++;
    const char *filename = &path[index];
    while (path[index] != '/') {
      if (index >= path_length) break;
      index++;
    }

    size_t filename_length = &path[index] - filename;
    if (filename_length == 0) {
      continue;
    }

    if (ext2_inode_read(extfs, current_inode_number, &current_inode)) {
      klog_error("failed inode read");
      return Error_Code_FAILED_READ;
    }
    
    if (ext2_find_inode_in_directory_from_name_string(extfs, &current_inode, 
        filename, filename_length, buffer, &current_inode_number)) {
      klog_error("failed to read inode could not find thingy");
      return Error_Code_VFS_NODE_NOT_FOUND;
    }
  }

  out_handle->fs_reference = current_inode_number;
  out_handle->file_size = (uint64_t)current_inode.size_low | ((uint64_t)current_inode.size_high << 32LL);
  out_handle->creation_time = current_inode.creation_time;
  out_handle->accessed_time = current_inode.last_access_time;
  out_handle->modified_time = current_inode.last_modification_time;
  return Error_Code_NONE;
}

#if 1
Error_Code vfs_node_iterate_directory(VFS_Node_Handle *handle, VFS_Node_Info_Procedure procedure, void *userdata) {
  Ext2_Filesystem *extfs = &globals.ext2_filesystem;
  uint8_t buffer[4096] = {};
  Ext2_Inode current_inode = {};
  if (ext2_inode_read(extfs, handle->fs_reference, &current_inode)) {
    return Error_Code_FAILED_READ;
  }

  uint64_t total_size = current_inode.size_low;
  total_size |= (uint64_t)current_inode.size_high << 32;
  if (total_size > 1UL << 63) {
    klog_error("file too large to read");
    return Error_Code_INVALID_DATA;
  }

  size_t blocks_to_read = total_size / extfs->block_size;
  if (total_size % extfs->block_size != 0) blocks_to_read++;
  size_t blocks_read_so_far = 0;
  //TODO(Torin: 2017-08-05) How to handle this correctly?
  if (blocks_to_read > 12) {
    klog_error("file too large to read");
    return Error_Code_UNSUPORTED_FEATURE;
  }

  while (blocks_read_so_far < blocks_to_read) {
    if (ext2_block_read(extfs, current_inode.direct_block_pointers[blocks_read_so_far], buffer)) {
      klog_error("failed to read directory entries");
      return Error_Code_FAILED_READ;
    }

    //TODO(Torin: 2017-08-05) Make sure end case is right
    Ext2_Directory_Entry *directory_entry = (Ext2_Directory_Entry *)buffer;
    while ((uintptr_t)directory_entry - (uintptr_t)buffer < extfs->block_size) {
      if (directory_entry->inode != 0) {
        VFS_Node_Info node_info = {};
        memory_copy(node_info.name, directory_entry->name, directory_entry->name_length);
        procedure(&node_info, userdata);
      }
      directory_entry = (Ext2_Directory_Entry *)((uintptr_t)directory_entry + directory_entry->entry_size);
    }

    blocks_read_so_far++;
  }

  return Error_Code_NONE;
}
#endif