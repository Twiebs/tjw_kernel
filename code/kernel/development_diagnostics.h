
#define development_only_runtime_assert(expression) development_assertion_handler(expression, #expression, __FILE__, __LINE__)
#define development_only_runtime_soft_error() development_soft_error_handler(__FILE__, __LINE__, __FUNCTION__)

static inline void development_assertion_handler(bool expression, const char *expression_string, const char *filename, int line_number) {
  if (expression == false) {
  	klog_error("ASSERTION FAILED[%s:%d] %s", filename, line_number, expression_string);
  	kernel_panic();
  }
}

static inline void development_soft_error_handler(const char *file, int line_number, const char *function_name) {
  klog_warning("[Soft Error] Failure at %s:%d in function %s", file, line_number, function_name);
}

