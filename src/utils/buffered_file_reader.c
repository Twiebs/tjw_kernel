
void buffered_file_reader_initialize(Buffered_File_Reader *reader, VFS_Node_Handle *handle) {
	reader->handle = *handle;
}

void buffered_file_reader_set_file_offset(Buffered_File_Reader *reader, uint64_t offset) {
  int64_t delta_offset = (int64_t)offset - reader->file_offset;
  int64_t new_buffer_offset = (int64_t)reader->buffer_offset + delta_offset;
  if (new_buffer_offset > 0 && new_buffer_offset < 512) {
  	reader->buffer_offset = new_buffer_offset;
  	reader->buffer_remaining += delta_offset;
  } else {
  	reader->buffer_offset = 0;
  	reader->buffer_remaining = 0;
  }

  reader->file_offset = offset;
}

uint64_t buffered_file_reader_read_bytes_to_buffer(Buffered_File_Reader *reader, void *buffer, size_t bytes) {
  uint64_t bytes_to_write_remaining = bytes;
  uint64_t total_bytes_written = 0;
  while (total_bytes_written < bytes) {
	  uint64_t bytes_to_write = min_uint64(bytes_to_write_remaining, reader->buffer_remaining);
	  memory_copy(buffer + total_bytes_written, reader->buffer + reader->buffer_offset, bytes_to_write);
    klog_debug("coppied %lu bytes!", bytes_to_write);
	  total_bytes_written += bytes_to_write;
	  bytes_to_write_remaining -= bytes_to_write;
	  reader->buffer_offset += bytes_to_write;
	  reader->buffer_remaining -= bytes_to_write;

	  if (total_bytes_written < bytes) {
      klog_debug("uhhhhh were reading again!");
	  	if (vfs_node_read_file(&reader->handle, reader->file_offset, 4, reader->buffer)) {
	  	  return total_bytes_written;
	  	}

	  	log_bytes(reader->buffer, 512);
	  	reader->buffer_offset = 0;
	  	reader->buffer_remaining = 512;
	  }
  }

  return total_bytes_written;
}