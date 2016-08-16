//NOTE(Torin) Supports only PCI access mechanisim #1
//TODO(Torin) Must write bootloader or somthing to go back to real mode
//and check bios for the access mechanism.  This just assumes that someone
//is not running on hardware from 1992 (this is a reasonable assumption for now)

#define PCI_CLASS_SERIAL_BUS_CONTROLLER 0x0C
#define PCI_SUBCLASS_USB_CONTROLLER 0x03

static inline
void unpack_32_2x16(uint32_t value, uint16_t *a, uint16_t *b){
  *a = (value >> 16) & 0xFFFF;
  *b = (value & 0xFFFF);
}

static inline
void unpack_32_4x8(uint32_t value, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d){
  *a = (value >> 24) & 0xFF;
  *b = (value >> 16) & 0xFF;
  *c = (value >>  8) & 0xFF;
  *d = (value >>  0) & 0xFF;
}

static inline
void pci_set_config_address(uint8_t bus_number, uint8_t device_number, uint8_t function_number, uint8_t register_number){
  struct PCI_Config_Adress {
    union {
      struct {
        uint8_t register_number;      //0-7
        uint8_t function_number : 3; //8 - 10
        uint8_t device_number   : 5; //11 - 15
        uint8_t bus_number; //16-23  
        uint8_t reserved : 7; //24-30
        uint8_t enable : 1; //31 
      };
      uint32_t packed;
    };
  } config_address;

  config_address.register_number = register_number;
  config_address.function_number = function_number;
  config_address.device_number = device_number;
  config_address.bus_number = bus_number;
  config_address.enable = 1;

  static const uint16_t PCI_CONFIG_ADDRESS_PORT = 0x0CF8;  
  write_port_uint32(PCI_CONFIG_ADDRESS_PORT, config_address.packed);
}

static inline
uint32_t pci_read_uint32(){
  static const uint16_t PCI_CONFIG_DATA_PORT = 0x0CFC;
  uint32_t result = read_port_uint32(PCI_CONFIG_DATA_PORT);
  return result;
}

static inline
void pci_read_2x16(uint16_t *a, uint16_t *b){
  uint32_t value = pci_read_uint32();
  unpack_32_2x16(value, a, b);
}

static inline
void pci_read_4x8(uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d){
  uint32_t value = pci_read_uint32();
  unpack_32_4x8(value, a, b, c, d);
}

static inline
void pci_log_device_info(uint8_t bus_number, uint8_t slot_number){
  pci_set_config_address(bus_number, slot_number, 0, 0);
  uint16_t vendor, device;
  pci_read_2x16(&vendor, &device);
  if(vendor == 0xFFFF) { return; }

  pci_set_config_address(bus_number, slot_number, 0, 0x08);
  uint8_t class_code, subclass, prog_if, revision_id;
  pci_read_4x8(&class_code, &subclass, &prog_if, &revision_id);
  if(class_code == PCI_CLASS_SERIAL_BUS_CONTROLLER && subclass == PCI_SUBCLASS_USB_CONTROLLER){
    klog_debug("PCI Slot: %u", (uint32_t)slot_number);
    klog_debug("  vendor: %u", (uint32_t)vendor);
    klog_debug("  device: %u", (uint32_t)device);
    klog_debug("  class_code: %u", (uint32_t)class_code);
    klog_debug("  subclass: %u", (uint32_t)subclass);
    klog_debug("  prog_if: %u", (uint32_t)prog_if);
  }
}

static inline
void pci_enumerate_devices(){
  for(size_t i = 0; i < 256; i++){
    for(size_t j = 0; j < 32; j++){
     pci_log_device_info(i, j);
    }
  }
}