#define CONSOLE_ENTRY_COUNT (1024)
#define CONSOLE_OUTPUT_BUFFER_SIZE (1 << 14)
#define CIRCULAR_LOG_MESSAGE_SIZE 256 
#define CIRCULAR_LOG_ENTRY_COUNT 256

#define LOG_CATEGORY_METALIST \
_(DEFAULT) \
_(DEBUG0) 

static const char * LOG_CATEGORY_NAMES[] = {
#define _(name) #name,
  LOG_CATEGORY_METALIST
#undef _
};

typedef enum {
#define _(name) Log_Category_##name,
  LOG_CATEGORY_METALIST
#undef _
  Log_Category_COUNT
} Log_Category;

typedef enum {
  Log_Category_State_ENABLED,
  Log_Category_State_DISABLED,
} Log_Category_State;

typedef struct {
  char message[CIRCULAR_LOG_MESSAGE_SIZE];
  uint32_t length;
} Circular_Log_Entry;

//TODO(Torin) Change is_dirty to last_event_timestamp
typedef struct {
  uint8_t category_states[Log_Category_COUNT];
  Circular_Log_Entry entries[CIRCULAR_LOG_ENTRY_COUNT];
  size_t entry_write_position;
  size_t current_entry_count;
  size_t scroll_offset;
  char input_buffer[256];
  size_t input_buffer_count;
  Spin_Lock spinlock;
  bool is_dirty;
} Circular_Log;

void klog_write_fmt(Circular_Log *log, Log_Category category, const char *fmt, ...);
void klog_enable_category(Circular_Log *log, Log_Category category);
void klog_disable_category(Circular_Log *log, Log_Category category);


void klog_write_string(Circular_Log *log, const char *string, size_t length);
void klog_add_input_character(Circular_Log *log, const char c);
void klog_remove_last_input_character(Circular_Log *log);
void klog_submit_input_to_shell(Circular_Log *log);
void klog_disable();
void klog_enable();