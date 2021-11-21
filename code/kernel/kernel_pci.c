//NOTE(Torin) Supports only PCI access mechanisim #1
//TODO(Torin) Must write bootloader or somthing to go back to real mode
//and check bios for the access mechanism.  This just assumes that someone
//is not running on hardware from 1992 (this is a reasonable assumption for now)

static inline void pci_set_config_address(uint8_t bus_number, uint8_t device_number, uint8_t function_number, uint8_t register_number) {
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
  write_port_uint32(PCI_CONFIG_ADDRESS_PORT, config_address.packed);
}



static inline uint32_t pci_read_uint32(){
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
  write_port_uint32(PCI_CONFIG_DATA_PORT, value);
}

void pci_device_config_address_set(PCI_Device *pci_device, uint8_t register_number) {
  pci_set_config_address(pci_device->bus_number, pci_device->device_number, pci_device->function_number, register_number);
}

uintptr_t pci_device_get_base_address_0(PCI_Device *pci_device) {
  pci_device_config_address_set(pci_device, 0x10);
  uint32_t base_address = pci_read_uint32();
  base_address &= 0xFFFFFFF0;
  return base_address;
}

uintptr_t pci_device_get_base_address_5(PCI_Device *pci_device) {
  pci_device_config_address_set(pci_device, 0x24);
  uint32_t base_address = pci_read_uint32();
  base_address &= 0xFFFFFFF0;
  return base_address;
}



Error_Code pci_ehci_initialize(PCI_Device *pci_device) {
  pci_device_config_address_set(pci_device, 0x10);
  uint32_t ehci_registers_address = pci_read_uint32();
  ehci_registers_address &= 0xFFFFFFF0;
  //TODO(Torin 2017-08-21) Proper error handling
  ehci_initalize_host_controller(ehci_registers_address, pci_device);
  return Error_Code_NONE;
}

PCI_Device *pci_device_create(uint8_t bus_number, uint8_t device_number, uint8_t function_number) {
  System_Info *system = &globals.system_info;
  PCI_Device *pci_device = NULL;
  if (system->pci_device_count >= ARRAY_COUNT(system->pci_devices)) {
    log_error(SYSTEM, "Failed to create pci device: Not enough storage in debug static array");
    return pci_device;
  }

  pci_device = &system->pci_devices[system->pci_device_count++];
  pci_device->bus_number = bus_number;
  pci_device->device_number = device_number;
  pci_device->function_number = function_number;
  return pci_device;
}

void pci_device_driver_create(uint8_t class_code, uint8_t subclass, uint8_t programming_interface,  uint16_t vendor_id, uint16_t device_id,
const char *type_description, PCI_Device_Initialization_Procedure initialization_procedure) {
  System_Info *system = &globals.system_info;
  if (system->pci_device_driver_count >= ARRAY_COUNT(system->pci_device_drivers)) {
    log_error(SYSTEM, "Failed to create pci device driver: Not enough storage in debug static array");
    return;
  }

  PCI_Device_Driver *pci_device_driver = &system->pci_device_drivers[system->pci_device_driver_count++];
  pci_device_driver->class_code = class_code;
  pci_device_driver->subclass = subclass;
  pci_device_driver->programming_interface = programming_interface;
  pci_device_driver->vendor_id = vendor_id;
  pci_device_driver->device_id = device_id;
  pci_device_driver->initialization_procedure = initialization_procedure;
  //TODO(Torin 2017-08-20) This is problably a bad idea to use static
  //strings since we might want to be able to  rehotload device
  //drivers in the event of a system update without requiring 
  //a full reboot of the os.
  pci_device_driver->type_description = type_description;
}

void pci_initialize_default_device_drivers() 
{
  // AHCI Advanced Host Controller Interface (SATA)
  pci_device_driver_create(PCI_Device_Class_MASS_STORAGE_CONTROLLER, 0x06, 0x01, 
    0x8086, 0x2829, "AHCI - Advanced Host Controller Interface (SATA)", ahci_initalize);

  //USB Host Controller Drivers
  pci_device_driver_create(PCI_Device_Class_SERIAL_BUS_CONTROLLER, PCI_SUBCLASS_USB_CONTROLLER,
    PCI_PROGRAMMING_INTERFACE_XHCI_CONTROLLER, 0xFFFF, 0x0000, "USB XHCI Host Controller", 0);
  pci_device_driver_create(PCI_Device_Class_SERIAL_BUS_CONTROLLER, PCI_SUBCLASS_USB_CONTROLLER,
    PCI_PROGRAMMING_INTERFACE_EHCI_CONTROLLER, 0xFFFF, 0x0000, "USB EHCI Host Controller", pci_ehci_initialize);
  pci_device_driver_create(PCI_Device_Class_SERIAL_BUS_CONTROLLER, PCI_SUBCLASS_USB_CONTROLLER,
    PCI_PROGRAMMING_INTERFACE_UHCI_CONTROLLER, 0xFFFF, 0x0000, "USB UHCI Host Controller", 0);
  pci_device_driver_create(PCI_Device_Class_SERIAL_BUS_CONTROLLER, PCI_SUBCLASS_USB_CONTROLLER,
    PCI_PROGRAMMING_INTERFACE_OHCI_CONTROLLER, 0xFFFF, 0x0000, "USB OHCI Host Controller", 0);

  //Graphics Device Drivers
  pci_device_driver_create(PCI_Device_Class_DISPLAY_CONTROLLER, 0x00, 0x00,
    0xFFFF, 0x0000, "VGA Compatible Graphics Device", 0);
  pci_device_driver_create(PCI_Device_Class_DISPLAY_CONTROLLER, 0x00, 0x00,
    0x8086, 0x0000, "Intel Graphics Device", intel_graphics_device_initialize);
  pci_device_driver_create(PCI_Device_Class_DISPLAY_CONTROLLER, 0x00, 0x00,
    0x1234, 0x1111, "Bochs Graphics Device", bochs_graphics_device_initialize);
}

void pci_device_set_type_description(PCI_Device *pci_device) {
  System_Info *system = &globals.system_info;
  for (size_t i = 0; i < system->pci_device_driver_count; i++) {
    PCI_Device_Driver *pci_device_driver = &system->pci_device_drivers[i];
    if (pci_device_driver->class_code == pci_device->class_code &&
        pci_device_driver->subclass == pci_device->subclass     &&
        pci_device_driver->programming_interface == pci_device->programming_interface) {
      pci_device->type_description = pci_device_driver->type_description;
      return;
    }
  }


  //NOTE(Torin 2017-08-20) The registered pci device drivers did not
  //contain a match for the pci_devices class, subclass, and programing interface
  //we now default to chosing a description string based on the class of the pci_device
  switch (pci_device->class_code) {
    case PCI_Device_Class_BEFORE_CLASS_CODE: pci_device->type_description = "Device created before class codes"; break;
    case PCI_Device_Class_MASS_STORAGE_CONTROLLER: pci_device->type_description = "Mass storage Controller"; break;
    case PCI_Device_Class_NETWORK_CONTROLLER: pci_device->type_description = "Network Controller"; break;
    case PCI_Device_Class_DISPLAY_CONTROLLER: pci_device->type_description = "Display Controller"; break;
    case PCI_Device_Class_MULTIMEDIA_CONTROLLER: pci_device->type_description = "Multimedia Controller"; break;
    case PCI_Device_Class_MEMORY_CONTROLLER: pci_device->type_description = "Memory Controller"; break;
    case PCI_Device_Class_BRIDGE_DEVICE: pci_device->type_description = "Bridge Device"; break;
    case PCI_Device_Class_SIMPLE_COMMUNICATION_CONTROLLER: pci_device->type_description = "Simple Communication Controller"; break;
    case PCI_Device_Class_BASE_SYSTEM_PERIPHERAL: pci_device->type_description = "Base System Peripheral"; break;
    case PCI_Device_Class_INPUT_DEVICE: pci_device->type_description = "Input Device"; break;
    case PCI_Device_Class_DOCKING_STATION: pci_device->type_description = "Docking Station"; break;
    case PCI_Device_Class_PROCESSOR: pci_device->type_description = "Processor"; break;
    case PCI_Device_Class_SERIAL_BUS_CONTROLLER: pci_device->type_description = "Serial Bus Controller"; break;
    case PCI_Device_Class_WIRELESS_CONTROLLER: pci_device->type_description = "Wireless Controller"; break;
    case PCI_Device_Class_INTELLIGENT_IO_CONTROLLER: pci_device->type_description = "Intelligent I/O Controller"; break;
    case PCI_Device_Class_SATELLITE_COMMUNICATION_CONTROLLER: pci_device->type_description = "Satellite Communication Controller"; break;
    case PCI_Device_Class_ENCRYPTION_CONTROLLER: pci_device->type_description = "Encryption Controller"; break;
    case PCI_Device_Class_DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER: pci_device->type_description = "Data Acquisition ans Signal Processing Controller"; break;
    case PCI_Device_Class_UNDEFINED: pci_device->type_description = "Undefined"; break;
    default:
      if (pci_device->class_code >= 0x12 && pci_device->class_code <= 0xFE) {
        pci_device->type_description = "Reserved";
      } else {
        pci_device->type_description = "INVALID PCI DEVICE CLASS";
      }
  };
}

void pci_debug_log_pci_device(PCI_Device *pci_device)
{
    log_debug(PCI, "PCI Device:");
    log_debug(PCI, "  %s", pci_device->type_description);
    log_debug(PCI, "  class_code: 0x%X", (uint64_t)pci_device->class_code);
    log_debug(PCI, "  subclass: 0x%X", (uint64_t)pci_device->subclass);
    log_debug(PCI, "  vendor_id: 0x%X", (uint64_t)pci_device->vendor_id);
    log_debug(PCI, "  device_id: 0x%X", (uint64_t)pci_device->device_id);
    log_debug(PCI, "  base_address_0:  0x%X", (uint64_t)pci_device->base_address_0);
    log_debug(PCI, "  base_address_1:  0x%X", (uint64_t)pci_device->base_address_1);
    log_debug(PCI, "  base_address_2:  0x%X", (uint64_t)pci_device->base_address_2);
    log_debug(PCI, "  base_address_3:  0x%X", (uint64_t)pci_device->base_address_3);
    log_debug(PCI, "  base_address_4:  0x%X", (uint64_t)pci_device->base_address_4);
    log_debug(PCI, "  base_address_5:  0x%X", (uint64_t)pci_device->base_address_5);
}


//NOTE(Torin 2017) When this procedure is called the pci_device
//already has its vendor_id and device_id information 
void pci_device_get_info(PCI_Device *pci_device) 
{
  pci_device_config_address_set(pci_device, 0x08);
  pci_read_4x8(&pci_device->class_code, &pci_device->subclass, 
    &pci_device->programming_interface, &pci_device->revision_id);

  uint16_t status, command;
  pci_device_config_address_set(pci_device, 0x04);
  pci_read_2x16(&status, &command);
  if (status & PCI_STATUS_CAPABILITIES_BIT) {
    pci_device->has_extended_capibilities = true;
  }

  uint8_t max_latency, min_grant, interrupt_pin, interrupt_line;
  pci_read_4x8(&max_latency, &min_grant, &interrupt_pin, &interrupt_line);
  pci_device_set_type_description(pci_device);

  pci_device_config_address_set(pci_device, 0x10);
  pci_device->base_address_0 = pci_read_uint32();
  pci_device->base_address_0 = pci_device->base_address_0 & 0xFFFFFFF0;
  pci_device_config_address_set(pci_device, 0x14);
  pci_device->base_address_1 = pci_read_uint32();
  pci_device->base_address_1 = pci_device->base_address_1 & 0xFFFFFFF0;
  pci_device_config_address_set(pci_device, 0x18);
  pci_device->base_address_2 = pci_read_uint32();
  pci_device->base_address_2 = pci_device->base_address_2 & 0xFFFFFFF0;
  pci_device_config_address_set(pci_device, 0x1C);
  pci_device->base_address_3 = pci_read_uint32();
  pci_device->base_address_3 = pci_device->base_address_3 & 0xFFFFFFF0;
  pci_device_config_address_set(pci_device, 0x20);
  pci_device->base_address_4 = pci_read_uint32();
  pci_device->base_address_4 = pci_device->base_address_4 & 0xFFFFFFF0;
  pci_device_config_address_set(pci_device, 0x24);
  pci_device->base_address_5 = pci_read_uint32();
  pci_device->base_address_5 = pci_device->base_address_5 & 0xFFFFFFF0;

  pci_debug_log_pci_device(pci_device);
}

void pci_enumerate_and_create_devices() {
  for (size_t bus_number = 0; bus_number < 256; bus_number++) {
    for (size_t device_number = 0; device_number < 32; device_number++) {
      size_t function_number = 0;
      size_t function_limit = 1;
      for (function_number = 0; function_number < function_limit; function_number++) {
        //First need to check if the device is valid by comparing the vendor ID to 0xFFFF
        uint16_t device = 0xFFFF, vendor = 0xFFFF;
        pci_set_config_address(bus_number, device_number, function_number, 0x00);
        pci_read_2x16(&device, &vendor);
        if (vendor == 0xFFFF) continue;
        PCI_Device *pci_device = pci_device_create(bus_number, device_number, function_number);
        pci_device->vendor_id = vendor;
        pci_device->device_id = device;
        pci_device_get_info(pci_device);

        uint8_t bist, header_type, latency_timer, cache_line_size;
        pci_set_config_address(bus_number, device_number, function_number, 0x0C);
        pci_read_4x8(&bist, &header_type, &latency_timer, &cache_line_size);
        static const uint8_t PCI_HEADER_TYPE_MULTIPLE_FUNCTIONS_BIT = 1 << 7;
        if (function_number == 0) {
          if (header_type & PCI_HEADER_TYPE_MULTIPLE_FUNCTIONS_BIT) function_limit = 8; 
        }
      }
    }
  }
}

PCI_Device_Driver *pci_find_matching_device_driver(PCI_Device *pci_device) {
  System_Info *system = &globals.system_info;
  PCI_Device_Driver *result = NULL;
  for (size_t i = 0; i < system->pci_device_driver_count; i++) {
    PCI_Device_Driver *pci_device_driver = &system->pci_device_drivers[i];
    if (pci_device_driver->class_code == pci_device->class_code &&
        pci_device_driver->subclass == pci_device->subclass     &&
        pci_device_driver->programming_interface == pci_device->programming_interface) {
      if (pci_device_driver->vendor_id == 0xFFFF && result == NULL) {
        result = pci_device_driver;
      } else if (pci_device_driver->vendor_id == pci_device->vendor_id &&
                 pci_device_driver->device_id == pci_device->device_id) {
        result = pci_device_driver;
      }
    }
  }

  return result;
}

Error_Code pci_initialize_device(PCI_Device *pci_device) {
  System_Info *system = &globals.system_info;
  PCI_Device_Driver *pci_device_driver = pci_find_matching_device_driver(pci_device);
  if (pci_device_driver == NULL) {
    return Error_Code_UNSUPORTED_FEATURE;
  }

  if (pci_device_driver->initialization_procedure != NULL) {
    pci_device_driver->initialization_procedure(pci_device);
  }

  return Error_Code_NONE;
}

void pci_initialize_valid_devices() {
  System_Info *system = &globals.system_info;
  for (size_t i = 0; i < system->pci_device_count; i++) {
    PCI_Device *pci_device = &system->pci_devices[i];
    pci_initialize_device(pci_device);
  }
}