

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  uint8_t  depth;
  uint8_t *buffer; 
} Framebuffer;

typedef struct {
  uint64_t glyph_size;
  uint8_t *data;
} Bitmap_Font;