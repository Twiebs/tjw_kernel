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

#define CIRCULAR_LOG_MESSAGE_SIZE 128
#define CIRCULAR_LOG_ENTRY_COUNT 256

typedef struct {
  char message[CIRCULAR_LOG_MESSAGE_SIZE];
  uint32_t length;
} Circular_Log_Entry;

typedef struct {
  Circular_Log_Entry entries[CIRCULAR_LOG_ENTRY_COUNT];
  size_t entry_write_position;
  size_t current_entry_count;
  size_t current_scroll_position;
  Spin_Lock spinlock;
} Circular_Log;

void klog_write_fmt(Circular_Log *log, const char *fmt, ...);
void klog_disable();
void klog_enable();