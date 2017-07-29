//NOTE(Torin) Supports only PCI access mechanisim #1
//TODO(Torin) Must write bootloader or somthing to go back to real mode
//and check bios for the access mechanism.  This just assumes that someone
//is not running on hardware from 1992 (this is a reasonable assumption for now)

#define PCI_CLASS_SERIAL_BUS_CONTROLLER 0x0C
#define PCI_SUBCLASS_USB_CONTROLLER 0x03


static inline void unpack_32_2x16(uint32_t value, uint16_t *a, uint16_t *b){
  *a = (value >> 16) & 0xFFFF;
  *b = (value & 0xFFFF);
}

static inline void unpack_32_4x8(uint32_t value, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d){
  *a = (value >> 24) & 0xFF;
  *b = (value >> 16) & 0xFF;
  *c = (value >>  8) & 0xFF;
  *d = (value >>  0) & 0xFF;
}

static inline void pci_set_config_address(uint8_t bus_number, uint8_t device_number, uint8_t function_number, uint8_t register_number){
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

static inline uint32_t pci_read_uint32(){
  const uint16_t PCI_CONFIG_DATA_PORT = 0x0CFC;
  uint32_t result = read_port_uint32(PCI_CONFIG_DATA_PORT);
  return result;
}

static inline void pci_read_2x16(uint16_t *a, uint16_t *b){
  uint32_t value = pci_read_uint32();
  unpack_32_2x16(value, a, b);
}

static inline void pci_read_4x8(uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d){
  uint32_t value = pci_read_uint32();
  unpack_32_4x8(value, a, b, c, d);
}

static inline void pci_write_uint32(volatile uint32_t value){
  const uint16_t PCI_CONFIG_DATA_PORT = 0x0CFC;
  write_port_uint32(PCI_CONFIG_DATA_PORT, value);
}

static inline void pci_scan_devices() {
  PCI_Info pci_info = {};

  for(size_t bus_number = 0; bus_number < 256; bus_number++){
    for(size_t device_number = 0; device_number < 32; device_number++){
      size_t function_number = 0;
      size_t function_limit = 1;
      
      uint16_t vendor, device;
      pci_set_config_address(bus_number, device_number, function_number, 0x00);
      pci_read_2x16(&vendor, &device);
      if(vendor == 0xFFFF) continue;

      uint8_t bist, header_type, latency_timer, cache_line_size;
      pci_set_config_address(bus_number, device_number, function_number, 0x0C);
      pci_read_4x8(&bist, &header_type, &latency_timer, &cache_line_size);
      static const uint8_t HEADER_TYPE_MULTIPLE_FUNCTIONS_BIT = 1 << 7;
      if(header_type & HEADER_TYPE_MULTIPLE_FUNCTIONS_BIT) function_limit = 8; 

      for (function_number = 0; function_number < function_limit; function_number++) {
        uint16_t vendor, device;
        pci_set_config_address(bus_number, device_number, function_number, 0x00);
        pci_read_2x16(&vendor, &device);
        if(vendor == 0xFFFF) continue;

        uint8_t class_code, subclass, prog_if, revision_id;
        pci_set_config_address(bus_number, device_number, function_number, 0x08);
        pci_read_4x8(&class_code, &subclass, &prog_if, &revision_id);

        uint16_t status, command;
        pci_set_config_address(bus_number, device_number, function_number, 0x04);
        pci_read_2x16(&status, &command);
        static const uint16_t STATUS_CAPABILITIES_BIT = 1 << 4;
        if (status & STATUS_CAPABILITIES_BIT) {
          klog_debug("pci has extended capabilities");
        }

        uint8_t max_latency, min_grant, interrupt_pin, interrupt_line;
        pci_read_4x8(&max_latency, &min_grant, &interrupt_pin, &interrupt_line);

        //NOTE(Torin 2016-09-13) Get the base addressess of the USB Host Controllers 
        if (class_code == PCI_CLASS_SERIAL_BUS_CONTROLLER && subclass == PCI_SUBCLASS_USB_CONTROLLER) {
          static const uint8_t UHCI_CONTROLLER = 0x00;
          static const uint8_t OHCI_CONTROLLER = 0x10;
          static const uint8_t EHCI_CONTROLLER = 0x20;
          static const uint8_t XHCI_CONTROLLER = 0x30;
          if (prog_if == UHCI_CONTROLLER) {
            klog_debug("[pci] Found UHCI Controller[%X:%X:%X]", bus_number, device_number, function_number);
            pci_set_config_address(bus_number, device_number, function_number, 0x20);
            uint32_t uhci_registers_address = pci_read_uint32(); 
            pci_info.uhci_physical_address = uhci_registers_address;
          } else if (prog_if == OHCI_CONTROLLER) {
            klog_debug("[pci] Found OHCI Controller[%X:%X:%X]", bus_number, device_number, function_number);
            pci_set_config_address(bus_number, device_number, function_number, 0x10);
            uint32_t ohci_bar_register = pci_read_uint32();
            uint32_t ohci_register_address = ohci_bar_register & ~0xFFF;
            pci_info.ohci_physical_address = ohci_register_address;
          } else if (prog_if == EHCI_CONTROLLER) { 
            pci_set_config_address(bus_number, device_number, function_number, 0x10);
            uint32_t ehci_registers_address = pci_read_uint32(); 
            pci_info.ehci_physical_address = ehci_registers_address;
            pci_info.ehci_pci_device.bus_number = bus_number;
            pci_info.ehci_pci_device.device_number = device_number;
            pci_info.ehci_pci_device.function_number = function_number;
            pci_info.ehci_pci_device.interrupt_pin = interrupt_pin;
            pci_info.ehci_pci_device.interrupt_line = interrupt_line;
            //TODO(Torin 2016-10-04) Initalizing the EHCI controller should be defered until
            //after PCI bus enumeration has completed.
            ehci_initalize_host_controller(pci_info.ehci_physical_address, &pci_info.ehci_pci_device);
          } else if (prog_if == XHCI_CONTROLLER) {
            klog_debug("[pci] Found XHCI Controller[%X:%X:%X]", bus_number, device_number, function_number);
            pci_set_config_address(bus_number, device_number, function_number, 0x10);
            uint32_t xhci_address_high = pci_read_uint32();
            pci_set_config_address(bus_number, device_number, function_number, 0x14);
            uint32_t xhci_address_low = pci_read_uint32();
            pci_info.xhci_physical_address = ((uintptr_t)xhci_address_high << 32);
            pci_info.xhci_physical_address |= xhci_address_low;
          }
        }

        //TODO(Torin 2016-09-16 Additional PCI device processing)
        if(0)
        {
        }

      } 
    }
  }
}
