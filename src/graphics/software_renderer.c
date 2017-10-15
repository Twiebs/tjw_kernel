
void software_graphics_buffer_create(Software_Graphics_Buffer *software_graphics_buffer, uint32_t width, uint32_t height) {
  software_graphics_buffer->width = width;
  software_graphics_buffer->height = height;
  software_graphics_buffer->stride = width * 4;
  size_t required_memory = width * height * 4;
  required_memory = memory_align(required_memory, 4096);
  size_t required_page_count = required_memory / 4096;
  software_graphics_buffer->data = memory_allocate_persistent_virtual_pages(required_page_count);
}

void software_graphics_buffer_draw_rectangle_solid(Software_Graphics_Buffer *software_graphics_buffer, int x, int y, int w, int h, Color_RGB_U8 color) {
  for (int yi = y; yi < y + h; yi++) {
    for (int xi = x; xi < x + w; xi++) {
      size_t pixel_index = (yi * software_graphics_buffer->stride) + (xi * 4);
      software_graphics_buffer->data[pixel_index + 0] = color.b;
      software_graphics_buffer->data[pixel_index + 1] = color.g;
      software_graphics_buffer->data[pixel_index + 2] = color.r;
    }
  }
}

void software_graphics_buffer_overlay_buffers(Software_Graphics_Buffer *dest, Software_Graphics_Buffer *src, uint8_t alpha) {
  kassert(dest->width == src->width);
  kassert(dest->height == src->height);
  for (size_t y = 0; y < dest->height; y++) {
    for (size_t x = 0; x < dest->width; x++) {
      size_t pixel_index = (y * dest->stride) + (x * 4);
      dest->data[pixel_index + 0] = src->data[pixel_index + 0] + (dest->data[pixel_index + 0] * (255 - alpha)) / 255;
      dest->data[pixel_index + 1] = src->data[pixel_index + 1] + (dest->data[pixel_index + 1] * (255 - alpha)) / 255;
      dest->data[pixel_index + 2] = src->data[pixel_index + 2] + (dest->data[pixel_index + 2] * (255 - alpha)) / 255;
    }
  }
}

void software_graphics_draw_character(Software_Graphics_Buffer *dest, const Bitmap_Font *font, char c, int x, int y) {
  const uint8_t *character_data = &font->data[(c - ' ') * 256];
  for (size_t i = 0; i < font->glyph_size; i++) {
    for(size_t j = 0; j < font->glyph_size; j++){
      size_t char_index = j + i * font->glyph_size; 
      if (character_data[char_index] > 0) {
        size_t pixel_index = ((j + x) * 4) + ((i + y) * dest->stride);
        dest->data[pixel_index + 0] = 0x00;
        dest->data[pixel_index + 1] = 0x00;
        dest->data[pixel_index + 2] = 0x00;
        dest->data[pixel_index + 3] = 0xFF;
      }
    }
  }
}

void software_graphics_draw_cstring(Software_Graphics_Buffer *dest, const Bitmap_Font *font, const char *cstring, int x, int y) {
  int current_x = x;
  size_t length = cstring_length(cstring);
  for (size_t i = 0; i < length; i++) {
    software_graphics_draw_character(dest, font, cstring[i], current_x, y);
    current_x += font->glyph_size / 2;
  }
}