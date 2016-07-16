
typedef struct {
	bool is_output_buffer_dirty;
	bool is_input_buffer_dirty;
	bool is_command_ready;
	char input_buffer[256];
	char output_buffer[1024*16];
	uint32_t input_buffer_count; 
	uint32_t output_buffer_count;
	uint32_t output_buffer_entry_count;
} IOState;

internal void stdout_write_fmt(IOState *io, const char *fmt, ...);
