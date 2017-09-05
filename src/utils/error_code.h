
typedef enum {
  Error_Code_NONE,
  Error_Code_FAILED_READ,
  Error_Code_INVALID_DATA,
  Error_Code_UNSUPORTED_FEATURE,
  Error_Code_VFS_NODE_NOT_FOUND,
  Error_Code_TIMEOUT,
} Error_Code;

const char *Error_Name[] = {
  "None",
  "Failed Read",
  "Invalid Data",
  "Unsupported Feature",
  "VFS_NODE_NOT_FOUND",
  "Timeout",
};