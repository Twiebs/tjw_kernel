
static const uint8_t INCONSOLATA16[] = {
#include "inconsolata16.txt"
};

void kgfx_draw_character(char c, size_t x_orign, size_t y_origin, Framebuffer *fb) {
  const uint8_t *character_data = &INCONSOLATA16[(c - ' ') * 256];
  for(size_t i = 0; i < 16; i++){
    for(size_t j = 0; j < 16; j++){
      size_t char_index = j + i*16; 
      if(character_data[char_index] > 0){
        size_t fb_index = ((j+x_orign)*fb->depth) + ((i+y_origin)*fb->pitch);
        fb->buffer[fb_index + 0] = 0x00;
        fb->buffer[fb_index + 1] = character_data[char_index];
        fb->buffer[fb_index + 2] = 0x00;
      }
    }
  }
}

void kgfx_clear_framebuffer(Framebuffer *fb){
  memset(fb->buffer, 0x00, fb->width * fb->height * fb->depth);
}



#if 0
void kgfx_draw_character(char c, size_t x_orign, size_t y_origin, Framebuffer *fb) {
  uint64_t character_bitmap = font[(size_t)c];
  uint64_t bitmask = (1L << 63);

  for(size_t i = 0; i < 8; i++){
    for(size_t j = 0; j < 8; j++){

      if(character_bitmap & bitmask){
        size_t y_begin = y_origin + i*1;
        size_t x_begin = x_orign + j*1;

        for(size_t y = y_begin; y < y_begin + 1; y++){
          for(size_t x = x_begin; x < x_begin + 1; x++){
            size_t index = x*fb->depth + y*fb->pitch;
            fb->buffer[index + 0] = 0x00;
            fb->buffer[index + 1] = 0xFF;
            fb->buffer[index + 2] = 0x00;
          }
        }
      }
      bitmask = bitmask >> 1;
    }
  }
}
#endif

static inline
void redraw_vga_text_terminal(VGA_Text_Terminal *kterm, Circular_Log *log) {
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	static uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);
	static const uint32_t VGA_TEXT_INDEX_MAX = 2000;

  if(log->current_entry_count < VGA_TEXT_ROW_COUNT){
    for(size_t i = 0; i < log->current_entry_count; i++){
     Circular_Log_Entry *entry = &log->entries[i]; 
     size_t chars_to_write = min(VGA_TEXT_COLUMN_COUNT, entry->length);
      for(size_t j = 0; j < chars_to_write; j++){
        size_t vga_index = ((i * VGA_TEXT_COLUMN_COUNT) + j) * 2;
        VGA_TEXT_BUFFER[vga_index+0] = entry->message[j];
        VGA_TEXT_BUFFER[vga_index+1] = VGAColor_GREEN;
      }
    }
  } else {
    size_t row_index = 0;
    for(int i = VGA_TEXT_ROW_COUNT - 1; i >= 0; i--){
      size_t entry_index = (log->current_scroll_position - i) % CIRCULAR_LOG_ENTRY_COUNT;
      Circular_Log_Entry *entry = &log->entries[entry_index];
      size_t chars_to_write = min(VGA_TEXT_COLUMN_COUNT, entry->length);
      for(size_t j = 0; j < chars_to_write; j++){
        size_t vga_index = ((row_index * VGA_TEXT_COLUMN_COUNT) + j) * 2;
        VGA_TEXT_BUFFER[vga_index+0] = entry->message[j];
        VGA_TEXT_BUFFER[vga_index+1] = VGAColor_GREEN;
      }

      int chars_to_clear = VGA_TEXT_COLUMN_COUNT - chars_to_write;
      for(int j = 0; j < chars_to_clear; j++){
        size_t vga_index = ((row_index * VGA_TEXT_COLUMN_COUNT) + j + chars_to_write) * 2;
        VGA_TEXT_BUFFER[vga_index + 0] = 0x00;
        VGA_TEXT_BUFFER[vga_index + 1] = 0x00;
      }

      row_index++;
    }
  }


  size_t chars_to_write = min(log->input_buffer_count, VGA_TEXT_COLUMN_COUNT);
  for(size_t i = 0; i < chars_to_write; i++){
    size_t vga_index = ((24 * VGA_TEXT_COLUMN_COUNT) + i) * 2;    
    VGA_TEXT_BUFFER[vga_index+0] = log->input_buffer[i];
    VGA_TEXT_BUFFER[vga_index+1] = VGAColor_RED;
  }

  size_t chars_to_clear = VGA_TEXT_COLUMN_COUNT - chars_to_write;
  for(size_t i = 0; i < chars_to_clear; i++){
    size_t vga_index = ((24 * VGA_TEXT_COLUMN_COUNT) + i + chars_to_write) * 2;    
    VGA_TEXT_BUFFER[vga_index+0] = 0x00; 
    VGA_TEXT_BUFFER[vga_index+1] = 0x00; 
  }
}

void redraw_log_if_dirty(Circular_Log *log){
  if(log->is_dirty == false) return;
  log->is_dirty = false;

  if(globals.framebuffer.buffer != 0){
    static const uint32_t FONT_SIZE = 16;
    static const uint32_t ROW_SPACING = 0;
    static const uint32_t CHARACTER_SPACING = 10;
    Framebuffer *fb = &globals.framebuffer;
    kgfx_clear_framebuffer(fb);
    const uint32_t total_column_count = fb->width / FONT_SIZE;
    const uint32_t max_row_count = fb->height / (FONT_SIZE + ROW_SPACING);
    const uint32_t total_lines_to_draw = min(log->current_entry_count, max_row_count - 1); 
    for(size_t i = 0; i < total_lines_to_draw; i++){
      size_t entry_offset = total_lines_to_draw - i; 
      size_t entry_index = (log->current_scroll_position - entry_offset) % CIRCULAR_LOG_ENTRY_COUNT;
      Circular_Log_Entry *entry = &log->entries[entry_index];  
      size_t chars_to_write = min(entry->length, total_column_count);
      for(size_t j = 0; j < chars_to_write; j++){
        kgfx_draw_character(entry->message[j], j*CHARACTER_SPACING, i*(FONT_SIZE + ROW_SPACING), fb);
      }
    }

    size_t input_buffer_to_write = min(total_column_count, log->input_buffer_count);
    for(size_t i = 0; i < input_buffer_to_write; i++){
      kgfx_draw_character(log->input_buffer[i], i*CHARACTER_SPACING, (FONT_SIZE+ROW_SPACING)*(max_row_count - 1), fb); 
    } 

  } else {
    redraw_vga_text_terminal(&globals.vga_text_term, log);
  }
}