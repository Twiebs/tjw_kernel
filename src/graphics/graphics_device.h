
typedef struct {

} Graphics_Device;

void graphics_device_initalize(Graphics_Device *graphics_device);
void graphics_device_memory_acquire();
void graphics_device_memory_release();
void graphics_device_memory_copy();
void graphics_device_memory_set();