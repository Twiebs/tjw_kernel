#define CONSOLE_ENTRY_COUNT (1024)
#define CONSOLE_OUTPUT_BUFFER_SIZE (1 << 14)
#define CIRCULAR_LOG_MESSAGE_SIZE 256 
#define CIRCULAR_LOG_ENTRY_COUNT 256

typedef enum {
  Log_Level_DEBUG,
  Log_Level_VERBOSE,
  Log_Level_INFO,
  Log_Level_WARNING,
  Log_Level_ERROR,
} Log_Level;

typedef enum {
  Log_Category_DEFAULT,
  Log_Category_DEBUG0,
  Log_Category_MEMORY,
  Log_Category_VFS,
  Log_Category_COUNT
} Log_Category;

static const char * LOG_CATEGORY_NAMES[] = {
  "Default",
  "Debug0",
  "Memory",
  "VFS",
  "__COUNT__"
};

typedef struct {
  Log_Level level;
  Log_Category category;
  uint64_t length;
  char message[CIRCULAR_LOG_MESSAGE_SIZE];
} Log_Entry;

//TODO(Torin) Change is_dirty to last_event_timestamp
typedef struct {
  Log_Entry entries[CIRCULAR_LOG_ENTRY_COUNT];
  uint64_t entries_front;
  uint64_t entries_back;
  Spin_Lock spinlock;
} Circular_Log;


void klog_write_fmt(Circular_Log *log, Log_Category category, Log_Level level, const char *fmt, ...);
//void klog_write_string(Circular_Log *log, const char *string, size_t length);

void klog_enable_category(Circular_Log *log, Log_Category category);
void klog_disable_category(Circular_Log *log, Log_Category category);

void klog_add_input_character(Circular_Log *log, const char c);
void klog_remove_last_input_character(Circular_Log *log);
void klog_submit_input_to_shell(Circular_Log *log);
void klog_disable();
void klog_enable();