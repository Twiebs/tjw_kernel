
static const uint8_t INCONSOLATA16[] = {
#include "INCONSOLATA16.txt"
};

#include <xmmintrin.h>

#if 0
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
}2

#else

void kgfx_draw_character(char c, size_t x_orign, size_t y_origin, Framebuffer *fb) {
  const uint8_t *character_data = &INCONSOLATA16[(c - ' ') * 256];
  for(size_t i = 0; i < 16; i++){
    for(size_t j = 0; j < 16; j++){
      size_t char_index = j + i*16; 
      if(character_data[char_index] > 0){
        size_t fb_index = ((j+x_orign)*fb->depth) + ((i+y_origin)*fb->pitch);
        fb->buffer[fb_index + 0] = 0x00;        fb->buffer[fb_index + 1] = character_data[char_index];
        fb->buffer[fb_index + 2] = 0x00;
      }
    }
  }
}
#endif

#if 0
void kgfx_clear_framebuffer(Framebuffer *fb){
  size_t step_count = (fb->width * fb->height * fb->depth) / 16;
  __m128i *write_ptr = (__m128i *)fb->buffer; 
  __m128i clear_value = _mm_setzero_si128();
  for(size_t i = 0; i < step_count; i++){
    _mm_store_si128(write_ptr, clear_value);
    write_ptr = (__m128i *)((uintptr_t)write_ptr + 16);
  }
}
#else
void kgfx_clear_framebuffer(Framebuffer *fb){
  memory_set(fb->buffer, 0x00, fb->width * fb->height * fb->depth);
}
#endif

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

void kgfx_draw_log_if_dirty(Circular_Log *log){
  if(log->is_dirty == false) return;
  log->is_dirty = false;

  if(globals.framebuffer.buffer != 0){
    static const uint32_t FONT_SIZE = 16;
    static const uint32_t ROW_SPACING = 0;
    static const uint32_t CHARACTER_SPACING = 10;
    Framebuffer *fb = &globals.framebuffer;
    kgfx_clear_framebuffer(fb);
    const uint32_t total_column_count = fb->width / CHARACTER_SPACING;
    const uint32_t max_row_count = fb->height / (FONT_SIZE + ROW_SPACING);
    const uint32_t total_lines_to_draw = min(log->current_entry_count, max_row_count - 1); 
    for(size_t i = 0; i < total_lines_to_draw; i++){
      size_t entry_offset = total_lines_to_draw - i; 
      size_t entry_index = (log->entry_write_position - entry_offset - log->scroll_offset) % CIRCULAR_LOG_ENTRY_COUNT;
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
    draw_vga_text_terminal(log);
  }
}

#endif