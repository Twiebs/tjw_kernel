#define CONSOLE_ENTRY_COUNT (1024)
#define CONSOLE_OUTPUT_BUFFER_SIZE (1 << 14)

#if 0
typedef enum {
  Console_Flag_OUTPUT_DIRTY = 1 << 0,
  Console_Flag_INPUT_DIRTY = 1 << 1,
  Console_Flag_COMMAND_READY = 1 << 2,
} Console_Flag;

typedef struct {
  uint32_t flags;
  uint32_t offset_of_entry[CONSOLE_ENTRY_COUNT];
  uint32_t length_of_entry[CONSOLE_ENTRY_COUNT];
  uint32_t current_entry_count;
  uint32_t entry_write_pos;
  uint32_t scroll_entry_index;
  char input_buffer[256];
  char output_buffer[CONSOLE_OUTPUT_BUFFER_SIZE];
} Console_Buffer;
#endif

#define CIRCULAR_LOG_MESSAGE_SIZE 256 
#define CIRCULAR_LOG_ENTRY_COUNT 256

typedef struct {
  char message[CIRCULAR_LOG_MESSAGE_SIZE];
  uint32_t length;
} Circular_Log_Entry;

//TODO(Torin) Change is_dirty to last_event_timestamp
typedef struct {
  Circular_Log_Entry entries[CIRCULAR_LOG_ENTRY_COUNT];
  size_t entry_write_position;
  size_t current_entry_count;
  size_t scroll_offset;
  char input_buffer[256];
  size_t input_buffer_count;
  Spin_Lock spinlock;
  bool is_dirty;
} Circular_Log;

void klog_write_fmt(Circular_Log *log, const char *fmt, ...);
void klog_write_string(Circular_Log *log, const char *string, size_t length);
void klog_add_input_character(Circular_Log *log, const char c);
void klog_remove_last_input_character(Circular_Log *log);
void klog_submit_input_to_shell(Circular_Log *log);
void klog_disable();
void klog_enable();