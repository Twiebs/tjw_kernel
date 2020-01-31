
static const uint16_t BOCHS_GRAPHICS_DEVICE_PORT_REGISTER = 0x01CE;
static const uint16_t BOCHS_GRAPHICS_DEVICE_PORT_DATA = 0x01CF;

static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_ID = 0;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_RESOLUTION_X = 1;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_RESOLUTION_Y = 2;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_BITS_PER_PIXEL = 3;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_ENABLE = 4;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_BANK = 5;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_VIRTUAL_WIDTH = 6;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_VIRTUAL_HEIGHT = 7;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_OFFSET_X = 8;
static const uint16_t BOCHS_GRAPHICS_DEVICE_REGISTER_OFFSET_Y = 9;

static const uint16_t BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_4 = 0x04;
static const uint16_t BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_8 = 0x08;
static const uint16_t BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_15 = 0x0F;
static const uint16_t BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_16 = 0x10;
static const uint16_t BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_24 = 0x18;
static const uint16_t BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_32 = 0x20;

static const uint16_t BOCHS_GRAPHICS_DEVICE_DISABLED = 0x00;
static const uint16_t BOCHS_GRAPHICS_DEVICE_ENABLED = 0x01;
static const uint16_t BOCHS_GRAPHICS_DEVICE_GET_CAPABILITIES = 0x02;
static const uint16_t BOCHS_GRAPHICS_DEVICE_ENABLED_LINEAR_FRAME_BUFFER = 0x40;

typedef struct {
  Graphics_Device base;
  uint16_t max_display_width;
  uint16_t max_display_height;
  uintptr_t framebuffer_memory_physical_address;
  Software_Graphics_Buffer framebuffer_a;
  Software_Graphics_Buffer framebuffer_b;
  Software_Graphics_Buffer *current_framebuffer;
} Bochs_Graphics_Device;

void bochs_graphics_device_write_register(uint16_t register_index, uint16_t value);
uint16_t bochs_graphics_device_read_register(uint16_t register_index);
Error_Code bochs_graphics_device_set_display_mode(Bochs_Graphics_Device *bochs_graphics_device, Display_Mode *display_mode);
Error_Code bochs_graphics_device_initialize(PCI_Device *pci_device);


void bochs_graphics_device_swap_buffers(Graphics_Device *graphics_device);
void bochs_graphics_device_get_back_buffer(Graphics_Device *graphics_device, Software_Graphics_Buffer *buffer);