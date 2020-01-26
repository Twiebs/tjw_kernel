
Log_Entry *klog_get_next_entry(Circular_Log *log) 
{
  size_t entry_index = log->entries_back % CIRCULAR_LOG_ENTRY_COUNT; 
  if (log->entries_back - log->entries_front > CIRCULAR_LOG_ENTRY_COUNT) 
  {
    log->entries_front += 1;
  }

  log->entries_back += 1;
  Log_Entry *entry = &log->entries[entry_index];
  return entry;
}

void klog_write_fmt(Circular_Log *log, Log_Category category, Log_Level level, const char *fmt, ...)
{
  if (globals.is_logging_disabled)
  {
    return;
  }

  Log_Entry *entry = klog_get_next_entry(log);
  entry->log_level = level;
  entry->log_category = category;

  va_list args;
  va_start(args, fmt);
  entry->message_length = vsnprintf(entry->message, LOG_ENTRY_MESSAGE_SIZE, fmt, args);
  va_end(args);

  write_serial(entry->message, entry->message_length);
  write_serial("\n", 1);
  globals.shell.requires_redraw = true;
}

void klog_disable()
{
  globals.is_logging_disabled = true;
}

void klog_enable()
{
  globals.is_logging_disabled = false;
}