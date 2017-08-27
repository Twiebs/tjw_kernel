
typedef struct {
  uint32_t width;
  uint32_t height;
  size_t stride;
  uint8_t *data;
} Graphics_Buffer;

typedef struct {
  Graphics_Buffer frame_buffer;
} Graphics_Device;

void graphics_device_initialize(Graphics_Device *graphics_device);
void graphics_device_memory_acquire();
void graphics_device_memory_release();
void graphics_device_memory_copy();
void graphics_device_memory_set();