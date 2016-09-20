
typedef struct {
  uint8_t capability_length;
  uint8_t reserved;
  uint16_t hci_version;
  uint32_t hcs_params;
  uint32_t hcc_params;
  uint64_t hcsp_port_route;
} EHCI_Capability_Registers;

typedef struct {
  volatile uint32_t usb_command;
  volatile uint32_t usb_status;
  volatile uint32_t usb_interrupt;
  volatile uint32_t frame_index;
  volatile uint32_t ctrl_ds_segment;
  volatile uint32_t perodic_list_base;
  volatile uint32_t async_list_address;
  volatile uint32_t reserved[9];
  volatile uint32_t config_flag;
  volatile uint32_t ports[0];
} __attribute((packed)) EHCI_Operational_Registers;

//32 bytes
typedef struct {
  uint32_t next_td;
  uint32_t alt_next_td;
  uint32_t token;
  uint32_t buffer_pointer[5];
} __attribute((packed)) EHCI_Transfer_Descriptor;

//48 bytes
typedef struct {
  uint32_t horizontal_link_pointer;
  uint32_t endpoint_characteristics;
  uint32_t endpoint_capabilities;
  uint32_t current_td;
  uint32_t next_td;
  uint32_t alt_next_td;
  uint32_t token;
  uint32_t buffer_pointer[5];
} EHCI_Queue_Head;

typedef struct {
  uint32_t periodic_frame_list[1024]; //4096 bytes
  //================================= PAGE BOUNDRAY
  EHCI_Transfer_Descriptor transfer_descriptors[4]; //4*32 = 128 bytes
  EHCI_Queue_Head asynch_qh;          //48 bytes
  EHCI_Queue_Head periodic_qh;        //48 bytes
} EHCI_Controller;

static EHCI_Controller g_ehci __attribute((aligned(4096)));

//====================================================================================

static inline
void kdebug_log_ehci_operational_registers(EHCI_Operational_Registers *opregs){
  klog_debug("usbcmd: 0x%X", (uint64_t)opregs->usb_command);
  klog_debug("usbstd: 0x%X", (uint64_t)opregs->usb_status);
  klog_debug("usbintr: 0x%X", (uint64_t)opregs->usb_interrupt);
  klog_debug("frindex: 0x%X", (uint64_t)opregs->frame_index);
}

//====================================================================================

static int 
ehci_check_qh_status(EHCI_Queue_Head *qh){
  static const uint32_t STATUS_ERROR_MASK = 0b01111100;
  static const uint32_t STATUS_HALTED = 1 << 6;
  static const uint32_t STATUS_DATA_BUFFER_ERROR = 1 << 5;
  static const uint32_t STATUS_ACTIVE = 1 << 7;
  const uint32_t error_status = qh->token & STATUS_ERROR_MASK;
  if(error_status != 0){
    klog_error("USB Transaction Error:"); 
    return -1;
  }
  return 1;
}

void ehci_control_transaction(){

}

static inline
void ehci_port_set(volatile uint32_t *port, uint32_t data){
  static const uint32_t PORT_CONNECT_STATUS_CHANGE = (1 << 1); 
  static const uint32_t PORT_ENABLE_DISABLE_CHANGE = (1 << 3);
  static const uint32_t PORT_OVER_CURRENT_CHANGE = (1 << 5);
  #define PORT_CHANGE_MASK (PORT_CONNECT_STATUS_CHANGE | PORT_ENABLE_DISABLE_CHANGE | PORT_OVER_CURRENT_CHANGE)
  uint32_t status = *port;
  status |= data;
  status &= ~PORT_CHANGE_MASK; *port = status;
  #undef PORT_CHANGE_MASK
}

static inline
void ehci_port_clear(volatile uint32_t *port, uint32_t data){
  static const uint32_t PORT_CONNECT_STATUS_CHANGE = (1 << 1); 
  static const uint32_t PORT_ENABLE_DISABLE_CHANGE = (1 << 3);
  static const uint32_t PORT_OVER_CURRENT_CHANGE = (1 << 5);
  #define PORT_CHANGE_MASK (PORT_CONNECT_STATUS_CHANGE | PORT_ENABLE_DISABLE_CHANGE | PORT_OVER_CURRENT_CHANGE)
  uint32_t status = *port;
  status &= ~PORT_CHANGE_MASK;
  status &= ~data;
  status |= PORT_CHANGE_MASK & data;
  *port = status;
  #undef PORT_CHANGE_MASK
}

static bool 
ehci_reset_port(volatile uint32_t *port){
  static const uint32_t EHCI_PORT_RESET_BIT = 1 << 8;
  static const uint32_t EHCI_PORT_ENABLED_BIT = 1 << 2;
  static const uint32_t EHCI_PORT_ENABLED_CHANGED_BIT = 1 << 3;

  *port = *port | EHCI_PORT_RESET_BIT;
  pit_wait_milliseconds(50); //NOTE(Torin 2016-09-17) Must Wait Max 50ms (USB2.0 spec 10.2.8.1)
  *port = *port & ~EHCI_PORT_RESET_BIT;
  while(*port & EHCI_PORT_RESET_BIT) {}
  pit_wait_milliseconds(2);

  if(*port & EHCI_PORT_ENABLED_BIT) return true;
  if(*port & EHCI_PORT_ENABLED_CHANGED_BIT){
    *port = *port & EHCI_PORT_ENABLED_CHANGED_BIT; 
    if(*port & EHCI_PORT_ENABLED_BIT) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}

void ehci_initalize(uintptr_t ehci_physical_address, PCI_Device *pci_device){
  klog_debug("starting ehci initalization...");

  //TODO(Torin 2016-09-16) Idealy this memory_map should occur in a different location
  uintptr_t ehci_virtual_page = 0x1000000;  
  uintptr_t page_offset = kmem_map_unaligned_physical_to_aligned_virtual_2MB(ehci_physical_address, ehci_virtual_page, 0); 
  uintptr_t ehci_virtual_address = page_offset + ehci_virtual_page;

  EHCI_Capability_Registers *cap_regs = (EHCI_Capability_Registers *)ehci_virtual_address;
  klog_debug("capabilites_length: %u", (uint32_t)cap_regs->capability_length);
  uintptr_t operational_register_base = ehci_virtual_address + cap_regs->capability_length;
  EHCI_Operational_Registers *op_regs = (EHCI_Operational_Registers *)operational_register_base;
  
  { //NOTE(Torin 2016-09-04) Extract information from hhcparams register
    static const uint32_t HHCPARAMS_EXT_CAPS_MASK = 0xFF00;
    static const uint32_t HHCPARAMS_ADDRESSING_BIT = (1 << 0);
    static const uint32_t ADDRESSING_CAPABILITY_32 = 0b00;
    static const uint32_t ADDRESSING_CAPABILITY_64 = 0b01;
    uint32_t addressing_capability = cap_regs->hcc_params & HHCPARAMS_ADDRESSING_BIT;
    uint32_t extended_capabilities = (cap_regs->hcc_params & HHCPARAMS_EXT_CAPS_MASK) >> 8;
    klog_debug("eecpp: 0x%X", (uint64_t)extended_capabilities);
    if(extended_capabilities == 0x00){
      klog_debug("ehci extended_capabilities not implemented");
    } else if (extended_capabilities < 0x40) {
      klog_error("inconsistant extended_capabilities pointer!");
    } else {
      static const uint32_t EHCI_USBLEGSUP_REGISTER_OFFSET = 0x00;
      uint32_t legacy_support_register = extended_capabilities + EHCI_USBLEGSUP_REGISTER_OFFSET;
      pci_set_config_address(pci_device->bus_number, pci_device->device_number, 
        pci_device->function_number, legacy_support_register);

      uint32_t legacy_support = pci_read_uint32();
      static const uint32_t USBLEGSUP_OS_OWNERSHIP = 1 << 24;
      static const uint32_t USBLEGSUP_BIOS_OWNERSHIP = 1 << 16;
      if(legacy_support & USBLEGSUP_BIOS_OWNERSHIP){
        klog_debug("taking ownership of usb controller");
        volatile uint32_t legacy_with_os_ownership = legacy_support | USBLEGSUP_OS_OWNERSHIP;
        pci_write_uint32(legacy_with_os_ownership);
        legacy_support = pci_read_uint32();
        while((legacy_support & USBLEGSUP_OS_OWNERSHIP) == 0)
          legacy_support = pci_read_uint32();
        klog_debug("set os owenership bit");
        legacy_support = pci_read_uint32();
        kassert(legacy_support & USBLEGSUP_OS_OWNERSHIP);
        while(legacy_support & USBLEGSUP_BIOS_OWNERSHIP)
          legacy_support = pci_read_uint32();
        klog_debug("the kernel now owns this ehci controller");
      }
    }

    //TODO(Torin 2016-09-16) Find out when it is appropiate to use 64bit data structures
    //for the EHCI operational data structures
    if(addressing_capability == ADDRESSING_CAPABILITY_32){
      klog_error("hhcparams does not support 64bit addressing"); 
    } else {
      klog_debug("usb host controller supports 64bit addressing");
    } 
  }

  size_t port_count = 0;
  bool is_port_power_control_enabled = false;
  { //NOTE(Torin 2016-09-17) Extract HCSPARAMS information
    static const uint32_t HCS_PORT_COUNT_MASK = 0b1111;
    static const uint32_t POWER_PORT_CONTROL_BIT = 1 << 4;
    port_count = cap_regs->hcs_params & HCS_PORT_COUNT_MASK; 
    is_port_power_control_enabled = cap_regs->hcs_params & POWER_PORT_CONTROL_BIT;
  }

  klog_debug("port count: %lu", port_count);
  klog_debug("is_port_power_control_enabled: %u", (uint32_t)is_port_power_control_enabled);

  { //NOTE(Torin 2016-09-06) Setup Queue heads and periodic frame list
    static const uint32_t EHCI_POINTER_TERMINATE = 1;
    static const uint32_t EHCI_POINTER_TYPE_QH = (1 << 1);
    EHCI_Queue_Head *asynch_qh = &g_ehci.asynch_qh;
    EHCI_Queue_Head *periodic_qh = &g_ehci.periodic_qh;
    memset(asynch_qh, 0x00, sizeof(EHCI_Queue_Head));
    memset(periodic_qh, 0x00, sizeof(EHCI_Queue_Head));

    //NOTE(Torin 2016-09-10) Setup the Asynch_Queue_Head
    asynch_qh->horizontal_link_pointer = ((uint32_t)(uintptr_t)(asynch_qh)) | EHCI_POINTER_TYPE_QH;
    asynch_qh->endpoint_characteristics = 0; //TODO(Torin 2016-09-06) This needs somthing
    asynch_qh->current_td = EHCI_POINTER_TERMINATE;
    asynch_qh->next_td = EHCI_POINTER_TERMINATE;
    asynch_qh->alt_next_td = EHCI_POINTER_TERMINATE;

    periodic_qh->horizontal_link_pointer = EHCI_POINTER_TERMINATE;
    periodic_qh->current_td = EHCI_POINTER_TERMINATE;
    periodic_qh->next_td = EHCI_POINTER_TERMINATE;
    periodic_qh->alt_next_td = EHCI_POINTER_TERMINATE;

    uint32_t *periodic_frame_list = g_ehci.periodic_frame_list;
    for(size_t i = 0; i < 1024; i++){
      periodic_frame_list[i] = (uint32_t)(uintptr_t)periodic_qh | EHCI_POINTER_TYPE_QH | EHCI_POINTER_TERMINATE;
    }
  }

  klog_debug("queue heads initalized");

    
  { //NOTE(Torin 2016-09-13) Set nessecary registers and initalize the controller 
    static const uint32_t USBCMD_RUN_STOP = (1 << 0);
    static const uint32_t USBCMD_HCRESET = 1 << 1;
    static const uint32_t USBCMD_PERIODIC_SCHEDULE_ENABLE = (1 << 4);
    static const uint32_t USBCMD_ASYNCH_SCHEDULE_ENABLE = (1 << 5);
    static const uint32_t USBCMD_INTERRUPT_THRESHOLD_CONTROL_8 = (0x08 << 16); //1ms
    static const uint32_t USBSTATUS_CONTROLLER_HALTED = (1 << 12);
    //NOTE(Torin 2016-09-04) Set all devices to be managed by this EHCI 
    static const uint32_t ROUTE_PORTS_TO_IMPLEMENTATION_DEPENDENT_CONTROLLER = 0b00;
    static const uint32_t ROUTE_PORTS_TO_HOST_EHCI = 0b01;

    op_regs->usb_command = op_regs->usb_command & (~USBCMD_RUN_STOP);
    while(op_regs->usb_command & USBCMD_RUN_STOP) { asm volatile ("nop"); }
    op_regs->usb_command = op_regs->usb_command | USBCMD_HCRESET;
    while(op_regs->usb_command & USBCMD_HCRESET) { asm volatile ("nop"); }

    op_regs->ctrl_ds_segment = 0;
    op_regs->usb_interrupt = 0;
    op_regs->perodic_list_base = (uint32_t)(uintptr_t)g_ehci.periodic_frame_list;
    op_regs->async_list_address = (uint32_t)(uintptr_t)&g_ehci.asynch_qh;
    op_regs->frame_index = 0;
    op_regs->config_flag = ROUTE_PORTS_TO_HOST_EHCI;
    op_regs->usb_command = USBCMD_INTERRUPT_THRESHOLD_CONTROL_8 | 
      USBCMD_PERIODIC_SCHEDULE_ENABLE | USBCMD_ASYNCH_SCHEDULE_ENABLE | USBCMD_RUN_STOP; 
    while((op_regs->usb_status & USBSTATUS_CONTROLLER_HALTED)) { asm volatile ("nop"); }
    klog_debug("ehci controller was started");
    op_regs->config_flag = ROUTE_PORTS_TO_HOST_EHCI;
  }

  { //NOTE(Torin 2016-09-06) Probe the ports managed by this controler
    klog_debug("enumerating %lu ports", port_count);
    for(size_t i = 0; i < port_count; i++){
      static const uint32_t PORT_ENABLED_BIT = (1 << 2);
      static const uint32_t PORT_POWER_BIT = 1 << 12;

      static const uint32_t PORT_ENABLED_STATUS_CHANGED_BIT = 1 << 3;
      static const uint32_t PORT_CONNECT_STATUS_CHANGED_BIT = 1 << 1;
      static const uint32_t PORT_ENALBED_STATUS_BIT = 1 << 2;
      static const uint32_t PORT_CONNECT_STATUS_BIT = 1 << 0;

      uint32_t volatile *port_reg = &op_regs->ports[i];
      if(*port_reg & PORT_CONNECT_STATUS_CHANGED_BIT){
        *port_reg = *port_reg | PORT_CONNECT_STATUS_CHANGED_BIT; //clears out connect status change
        while(*port_reg & PORT_CONNECT_STATUS_CHANGED_BIT) { asm volatile("nop"); } 
        if(*port_reg & PORT_CONNECT_STATUS_BIT){
          klog_debug("USB_Device connected on port %lu", i);
          bool is_port_enabled = *port_reg & PORT_ENABLED_BIT;
          if(is_port_enabled == false) is_port_enabled = ehci_reset_port(port_reg);
          if(is_port_enabled == false){
            klog_debug("Port %lu failed to enable", i);
          } else {
            klog_debug("Port %lu enabled sucuessfuly", i);

            //DO USB THINGS!!!!

          }
        }
      }
    }
  }
  klog_debug("echi was initalized");

  kgfx_draw_log_if_dirty(&globals.log);
}
