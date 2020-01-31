
typedef struct {
  uint32_t width;
  uint32_t height;
} Graphics_Display;

struct Graphics_Device;
typedef void(*Graphics_Device_Get_Back_Buffer)(struct Graphics_Device *, Software_Graphics_Buffer *);
typedef void(*Graphics_Device_Swap_Buffers)(struct Graphics_Device *);

typedef struct Graphics_Device {
  uint32_t width;
  uint32_t height;
  Graphics_Device_Get_Back_Buffer get_back_buffer;
  Graphics_Device_Swap_Buffers swap_buffers;
} Graphics_Device;

void graphics_device_initialize(Graphics_Device *graphics_device);
void graphics_device_memory_acquire();
void graphics_device_memory_release();
void graphics_device_memory_copy();
void graphics_device_memory_set();


static inline void graphics_device_swap_buffers(Graphics_Device *graphics_device) {
  graphics_device->swap_buffers(graphics_device);
}

static inline void graphics_device_get_back_buffer(Graphics_Device *graphics_device, Software_Graphics_Buffer *software_graphics_buffer) {
  graphics_device->get_back_buffer(graphics_device, software_graphics_buffer); 
}