
typedef enum {
  Error_Code_NONE,
  Error_Code_FAILED_READ,
  Error_Code_INVALID_DATA,
  Error_Code_UNSUPORTED_FEATURE,
  Error_Code_VFS_NODE_NOT_FOUND,
  Error_Code_TIMEOUT,
  Error_Code_HARDWARE_TIMEOUT,
  Error_Code_HARDWARE_FEATURE_UNAVAILABLE,
  Error_Code_OUT_OF_MEMORY,
  Error_Code_INVALID_PROCEDURE_PARAMETERS,
} Error_Code;

const char *Error_Name[] = {
  "None",
  "Failed Read",
  "Invalid Data",
  "Unsupported Feature",
  "VFS_NODE_NOT_FOUND",
  "Timeout",
  "Error_Code_HARDWARE_TIMEOUT",
  "Error_Code_HARDWARE_FEATURE_UNAVAILABLE",
  "Error_Code_OUT_OF_MEMORY",
  "Error_Code_INVALID_PROCEDURE_PARAMETERS",
};