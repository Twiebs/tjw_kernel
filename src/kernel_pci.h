#pragma once
#include <stdint.h>

static const uint16_t PCI_CONFIG_ADDRESS_PORT = 0x0CF8;  
static const uint16_t PCI_CONFIG_DATA_PORT = 0x0CFC;

static const uint8_t PCI_SUBCLASS_USB_CONTROLLER = 0x03;



static const uint16_t PCI_STATUS_CAPABILITIES_BIT = 1 << 4;


static const uint8_t PCI_PROGRAMMING_INTERFACE_UHCI_CONTROLLER = 0x00;
static const uint8_t PCI_PROGRAMMING_INTERFACE_OHCI_CONTROLLER = 0x10;
static const uint8_t PCI_PROGRAMMING_INTERFACE_EHCI_CONTROLLER = 0x20;
static const uint8_t PCI_PROGRAMMING_INTERFACE_XHCI_CONTROLLER = 0x30;



typedef enum {
  PCI_Device_Class_BEFORE_CLASS_CODE                                  = 0x00, //Device was built prior definition of the class code field 
  PCI_Device_Class_MASS_STORAGE_CONTROLLER                            = 0x01,
  PCI_Device_Class_NETWORK_CONTROLLER                                 = 0x02,
  PCI_Device_Class_DISPLAY_CONTROLLER                                 = 0x03,
  PCI_Device_Class_MULTIMEDIA_CONTROLLER                              = 0x04,
  PCI_Device_Class_MEMORY_CONTROLLER                                  = 0x05,
  PCI_Device_Class_BRIDGE_DEVICE                                      = 0x06,
  PCI_Device_Class_SIMPLE_COMMUNICATION_CONTROLLER                    = 0x07,
  PCI_Device_Class_BASE_SYSTEM_PERIPHERAL                             = 0x08,
  PCI_Device_Class_INPUT_DEVICE                                       = 0x09,
  PCI_Device_Class_DOCKING_STATION                                    = 0x0A,
  PCI_Device_Class_PROCESSOR                                          = 0x0B,
  PCI_Device_Class_SERIAL_BUS_CONTROLLER                              = 0x0C,
  PCI_Device_Class_WIRELESS_CONTROLLER                                = 0x0D,
  PCI_Device_Class_INTELLIGENT_IO_CONTROLLER                          = 0x0E,
  PCI_Device_Class_SATELLITE_COMMUNICATION_CONTROLLER                 = 0x0F,
  PCI_Device_Class_ENCRYPTION_CONTROLLER                              = 0x10,
  PCI_Device_Class_DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER  = 0x11,
  PCI_Device_Class_UNDEFINED                                          = 0xFF,
} PCI_Device_Class;

typedef enum {
  PCI_Device_Type_INVALID,
  PCI_Device_Type_UHCI,
  PCI_Device_Type_OHCI,
  PCI_Device_Type_EHCI,
  PCI_Device_Type_XHCI,
} PCI_Device_Type;




typedef struct {
  uint8_t bus_number;
  uint8_t device_number;
  uint8_t function_number;
  bool has_extended_capibilities;


  uint16_t vendor_id;
  uint16_t device_id;

  uint8_t class_code;
  uint8_t subclass;
  uint8_t programming_interface;
  uint8_t revision_id;  

  uint8_t interrupt_pin;
  uint8_t interrupt_line;

  bool is_initalized;
  const char *type_description; //Staticly allocated string
} PCI_Device;

typedef Error_Code(*PCI_Device_Initialization_Procedure)(PCI_Device *);

typedef struct {
  uint8_t class_code;
  uint8_t subclass;
  uint8_t programming_interface;
  const char *type_description;
  PCI_Device_Initialization_Procedure initialization_procedure;
} PCI_Device_Driver;


PCI_Device *pci_device_create(uint8_t bus_number, uint8_t device_number, uint8_t function_number);
void pci_device_driver_create(uint8_t class_code, uint8_t subclass, uint8_t programming_interface, 
  const char *type_description, PCI_Device_Initialization_Procedure initialization_procedure);

void pci_device_set_type_description(PCI_Device *pci_device);
void pci_device_get_info(PCI_Device *pci_device);

void pci_initialize_default_device_drivers();
void pci_enumerate_and_create_devices();
void pci_initialize_valid_devices();

Error_Code pci_ehci_initialize(PCI_Device *pci_device);