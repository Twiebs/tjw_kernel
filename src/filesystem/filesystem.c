

int fs_read_file(File_Handle *h, uint64_t offset, uint64_t size, uintptr_t *physical_pages){
  if((offset + size) > h->file_size) return 0;
  if(physical_pages == 0) return 0;

  Ext2_Inode inode = {}; 
  Ext2_Filesystem *extfs = &globals.ext2_filesystem;
  ext2fs_read_inode(extfs, h->fs_reference, &inode);
  if(inode.type != EXT2_INODE_TYPE_REGULAR_FILE){
    klog_error("failed to read file: INODE DOES NOT REFERENCE VALID FILE");
    return 0;
  }

  uint64_t pages_to_read = size / 4096;
  if(size % 4096 != 0) pages_to_read += 1;
  if(pages_to_read > 12) {
    klog_error("unsupported filesize!");
    return 0;
  }

  for(size_t i = 0; i < pages_to_read; i++){
    if(ext2fs_read_block(extfs, inode.direct_block_pointers[i], physical_pages[i]) == 0){
      klog_error("failed to read ext2 block");
      return 0;
    }
  } 

  return 1;
}

int fs_obtain_file_handle(const char *path, uint64_t path_length, File_Handle *in_handle){
  Ext2_Filesystem *extfs = &globals.ext2_filesystem; 
  if (path[0] != '/') {
    klog_error("malformed path.  Path's must begin with a '/'");
    return 0;
  }

  size_t index = 0;
  uint8_t buffer[4096] = {};
  Ext2_Inode current_inode = {};
  uint32_t inode_number = 2;
  ext2fs_read_inode(extfs, 2, &current_inode); 
  while(index < path_length){
    if(path[index] != '/'){
      klog_error("malformed path");
    }

    index++;
    const char *filename = &path[index];
    while(path[index] != '/'){
      if(index >= path_length) break;
      index++;
    }

    size_t filename_length = &path[index] - filename;
    klog_debug("searching ext2fs for file: %.*s", filename_length, filename);
    if(current_inode.type == EXT2_INODE_TYPE_DIRECTORY){
      uint16_t current_directory_index = 0;
      if(ext2fs_read_block(extfs, current_inode.direct_block_pointers[0], (uintptr_t)buffer) == 0){
        klog_error("failed to read directory entries");
        return 0;
      }

      Ext2_Directory_Entry *directory_entry = (Ext2_Directory_Entry *)buffer;
      bool found_file = false;
      while(((uintptr_t)directory_entry - (uintptr_t)buffer) < 4096 && (current_directory_index < current_inode.hard_link_count)){
        if(directory_entry->entry_size == 0) break;
        if(string_equals_string(directory_entry->name, directory_entry->name_length, filename, filename_length )){
          //kdebug_ext2_log_directory_entry(extfs, directory_entry);
          inode_number = directory_entry->inode;
          if(ext2fs_read_inode(extfs, directory_entry->inode, &current_inode) == 0){
            klog_error("failed to read inode");
            return 0;
          }

          //TODO(Torin) Do consitancy checks to ensure that this is a valid inode

          found_file = true;
          break;
        }
        directory_entry = (Ext2_Directory_Entry *)((uintptr_t)directory_entry + directory_entry->entry_size);
      } 

      if(found_file){
        klog_debug("found inode: %.*s", filename_length, filename); 
        //kdebug_ext2_log_inode(&current_inode);
      } else {
        klog_error("failed to find %.*s", filename_length, filename);
      }

      if(current_inode.type == EXT2_INODE_TYPE_REGULAR_FILE && index == path_length){
        in_handle->fs_reference = inode_number;
        in_handle->file_size = (uint64_t)current_inode.size_low | ((uint64_t)current_inode.size_high << 32LL);
        in_handle->creation_time = current_inode.creation_time;
        in_handle->accessed_time = current_inode.last_access_time;
        in_handle->modified_time = current_inode.last_modification_time;
        return 1;
      }

    } else {
        klog_error("%.*s is not a regular file!", filename_length, filename); 
        return 0;
    }
  } 
  return 0;
}