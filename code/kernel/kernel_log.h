
#define LOG_COMPILE_TIME_VERBOSITY_ERROR 0
#define LOG_COMPILE_TIME_VERBOSITY_INFO 1
#define LOG_COMPILE_TIME_VERBOSITY_DEBUG 2

#define LOG_COMPILE_TIME_VERBOSITY LOG_COMPILE_TIME_VERBOSITY_DEBUG

#if LOG_COMPILE_TIME_VERBOSITY >= LOG_COMPILE_TIME_VERBOSITY_DEBUG
//#define klog_debug(...) klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_DEBUG, "[%s:%u]", __FILE__, (uint32_t)__LINE__); klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_DEBUG, __VA_ARGS__)
#define klog_debug(...) klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_DEBUG, __VA_ARGS__)
#else //LOG_COMPILE_TIME_VERBOSITY >= LOG_COMPILE_TIME_VERBOSITY_DEBUG
#define klog_debug(...)
#endif //LOG_COMPILE_TIME_VERBOSITY >= LOG_COMPILE_TIME_VERBOSITY_DEBUG
#define klog_info(...) klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_INFO, __VA_ARGS__)
#define klog_warning(...) klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_WARNING, __VA_ARGS__)
#define klog_error(...) klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_ERROR, __VA_ARGS__)

#define log_enable(category) klog_enable_category(&globals.log, Log_Category_##category)
#define log_disable(category) klog_disable_category(&globals.log, Log_Category_##category)

#define log_error(category, ...) klog_write_fmt(&globals.log, Log_Category_##category, Log_Level_ERROR, __VA_ARGS__)
#define log_debug(category, ...) klog_write_fmt(&globals.log, Log_Category_##category, Log_Level_DEBUG, __VA_ARGS__)
#define log_info(category, ...) klog_write_fmt(&globals.log, Log_Category_##category, Log_Level_INFO, __VA_ARGS__)


#define CONSOLE_ENTRY_COUNT (1024)
#define CONSOLE_OUTPUT_BUFFER_SIZE (1 << 14)
#define CIRCULAR_LOG_ENTRY_COUNT 1024

typedef enum {
  Log_Level_DEBUG,
  Log_Level_VERBOSE,
  Log_Level_INFO,
  Log_Level_WARNING,
  Log_Level_ERROR,
} Log_Level;

typedef enum 
{
    Log_Category_DEFAULT,
    Log_Category_DEBUG0,
    Log_Category_INITIALIZATION,
    Log_Category_MEMORY,
    Log_Category_VFS,
    Log_Category_SYSTEM,
    Log_Category_ACPI,
    Log_Category_PCI,
    Log_Category_EHCI,
    Log_Category_AHCI,
    Log_Category_DESKTOP,
    Log_Category_COUNT
} Log_Category;

static const char* LOG_CATEGORY_NAMES[] = 
{
    "Default",
    "Debug0",
    "Initialization",
    "Memory",
    "VFS",
    "System",
    "ACPI",
    "PCI",
    "EHCI",
    "AHCI",
    "DESKTOP",
    "__COUNT__"
};

static const char* LOG_CATEGORY_TAGS[] = 
{
    "[Default] ",
    "[Debug0] ",
    "[Initialization] ",
    "[Memory] ",
    "[VFS] ",
    "[System] ",
    "[ACPI] ",
    "[PCI] ",
    "[EHCI] ",
    "[AHCI] ",
    "[Desktop] "
};

// log_level + log_category + message_length = 4 bytes
#define LOG_ENTRY_MESSAGE_SIZE  (256 - 4)

typedef struct {
  uint8_t log_level;
  uint8_t log_category;
  uint16_t message_length;
  char message[LOG_ENTRY_MESSAGE_SIZE];
} Log_Entry;

static_assert(sizeof(Log_Entry) == 256);

// This thing needs to be statically allocated because we want to be able to
// log things before the memory manager is initialized. Since it will be statically allocated
// I wanted to have a well-defined size that is 4096 page aligned.
typedef struct {
  Log_Entry entries[CIRCULAR_LOG_ENTRY_COUNT]; // 256 * 1024 = 262144 = 64 pages
  // 64 pages
  uint8_t padding[4080];
  uint64_t entries_front;
  uint64_t entries_back;
  // 65 pages
} Circular_Log;

static_assert((sizeof(Circular_Log) % 4096) == 0);
static_assert((sizeof(Circular_Log) == (4096 * 65)));

void klog_write_fmt(Circular_Log *log, Log_Category category, Log_Level level, const char *fmt, ...);
//void klog_write_string(Circular_Log *log, const char *string, size_t length);

void klog_enable_category(Circular_Log *log, Log_Category category);
void klog_disable_category(Circular_Log *log, Log_Category category);

void klog_add_input_character(Circular_Log *log, const char c);
void klog_remove_last_input_character(Circular_Log *log);
void klog_submit_input_to_shell(Circular_Log *log);
void klog_disable();
void klog_enable();
