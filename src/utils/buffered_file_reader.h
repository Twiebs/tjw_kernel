
typedef struct {
  VFS_Node_Handle handle;
  uint64_t file_offset;
  uint64_t buffer_offset;
  uint64_t buffer_remaining;
  uint8_t buffer[512];
} Buffered_File_Reader;

void buffered_file_reader_initialize(Buffered_File_Reader *reader, VFS_Node_Handle *handle);
void buffered_file_reader_set_file_offset(Buffered_File_Reader *reader, uint64_t offset);
uint64_t buffered_file_reader_read_bytes_to_buffer(Buffered_File_Reader *reader, void *buffer, size_t bytes);