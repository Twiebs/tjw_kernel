#define static_assert(expr,msg) _Static_assert(expr,msg)
#define wait_for_condition(x) { volatile uint64_t counter = 0; while((!(x)) && counter < 0xFFFFFFF) { counter++; } if(counter == 0xFFFFFFF) { klog_debug("wait timed out: %s,  %s:%u", #x, __FILE__, __LINE__); }}

typedef struct {
  volatile uint8_t capability_length;
  volatile uint8_t reserved;
  volatile uint16_t hci_version;
  volatile uint32_t hcs_params;
  volatile uint32_t hcc_params;
  volatile uint64_t hcsp_port_route;
} __attribute((packed)) EHCI_Capability_Registers;

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

typedef struct {
  volatile uint8_t usb_interrupt : 1; // 0
  volatile uint8_t usb_error_interrupt : 1; //1
  volatile uint8_t port_change_detect : 1; //2
  volatile uint8_t frame_list_rollover : 1; //3
  volatile uint8_t host_system_error : 1; // 4
  volatile uint8_t interrupt_on_async_advance : 1; //5
  volatile uint8_t reserved0 : 2; //6-7
  volatile uint8_t reserved1 : 4; //8-11
  volatile uint8_t hc_halted : 1; //12
  volatile uint8_t reclamation : 1; //13
  volatile uint8_t perodic_schedule_status : 1; //14
  volatile uint8_t asynch_schedule_status : 1; //15
  volatile uint16_t reserved2; //16-31  
} __attribute((packed)) EHCI_USB_Status_Register;

typedef struct {
  volatile uint8_t ping_state : 1; //0
  volatile uint8_t split_transaction_state : 1; //1
  volatile uint8_t missed_micro_frame : 1; //2
  volatile uint8_t transaction_error : 1; //3
  volatile uint8_t babble_detected : 1; //4
  volatile uint8_t data_buffer_error : 1; //5
  volatile uint8_t halted : 1; //6
  volatile uint8_t active : 1; //7
  volatile uint8_t packet_id : 2; //8-9
  volatile uint8_t error_counter : 2; //10-11
  volatile uint8_t current_page : 3; //12-14
  volatile uint8_t interrupt_on_complete : 1; //15
  volatile uint16_t total_bytes_to_transfer : 15; //16-30
  volatile uint8_t data_toggle: 1; //31
} __attribute((packed)) EHCI_QTD_Token;

//NOTE(Torin 2016-10-03) The QTD and QH DS are used in the 64 and 32 bit variants of the
//EHCI spec data structures
typedef struct {
  volatile uint32_t next_td;
  volatile uint32_t alt_next_td;
  volatile uint32_t qtd_token;
  volatile uint32_t buffer_pointer_low[5];
  volatile uint32_t buffer_pointer_high[5];
  volatile uint8_t padding[12]; //Force 64byte size for cache line alignment
} __attribute((packed)) EHCI_QTD; //64 bytes

typedef struct {
  volatile uint32_t horizontal_link_pointer;
  volatile uint32_t endpoint_characteristics;
  volatile uint32_t endpoint_capabilities;
  volatile uint32_t current_td;
  volatile uint32_t next_td;
  volatile uint32_t alt_next_td;
  volatile uint32_t qtd_token;
  volatile uint32_t buffer_pointer_low[5];
  volatile uint32_t buffer_pointer_high[5];
} __attribute((packed)) EHCI_Queue_Head; //68 Bytes

typedef struct {
  volatile uint8_t current_connect_status : 1; //0
  volatile uint8_t connect_status_change : 1;  //1
  volatile uint8_t port_enabled : 1; //2
  volatile uint8_t port_enabled_change: 1; //3
  volatile uint8_t over_current_active: 1; //4
  volatile uint8_t over_current_change: 1; //5
  volatile uint8_t force_port_resume: 1; //6
  volatile uint8_t suspend: 1; //7
  volatile uint8_t port_reset: 1; //8
  volatile uint8_t reserved0 : 1; //9
  volatile uint8_t line_status :2; //10-11
  volatile uint8_t port_power : 1; //12
  volatile uint8_t port_owner : 1; // 13
  volatile uint8_t port_indicator_control : 2; //14-15
  volatile uint8_t port_test_control : 4; //16-19
  volatile uint8_t wake_on_connect_enable : 1; //20
  volatile uint8_t wake_on_disconnect_enable : 1; //21
  volatile uint8_t wake_on_over_current_enable : 1; //22
  volatile uint16_t reserved1 : 9; // 23-31
} __attribute((packed)) EHCI_Port;

static_assert(sizeof(EHCI_Port) == 4, "EHCI_Port Struct has Invalid Packing");

//=======================================================================================

typedef struct {
  uint32_t periodic_frame_list[1024]; //4096 bytes
  //====================================================== PAGE BOUNDRAY
  EHCI_Queue_Head asynch_qh __attribute((aligned(64))); //68 bytes
  EHCI_Queue_Head periodic_qh __attribute((aligned(64))); //68 bytes
  //TODO(Torin 2016-09-25) EHCI spec says these structs need to be "32-byte (cache line)"
  //aligned.  Does this mean they need to be 64Bytes aligned on a newer system? They are
  //already bigger than 64 bytes so I will just put that for now 

  EHCI_QTD qtd_array[4] __attribute((aligned(64))); //4*64 = 256 bytes
  EHCI_Operational_Registers *op_regs;
  PCI_Device pci_device;
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

static inline
void kdebug_log_qtd_token(const uint32_t in_token){
  EHCI_QTD_Token qtd_token = *(EHCI_QTD_Token *)(&in_token);
  klog_debug(" split_transaction_state: %u", (uint32_t)qtd_token.split_transaction_state);
  klog_debug(" missed_micro_frame: %u", (uint32_t)qtd_token.missed_micro_frame);
  klog_debug(" transaction_error: %u", (uint32_t)qtd_token.transaction_error);
  klog_debug(" babble_detected: %u", (uint32_t)qtd_token.babble_detected);
  klog_debug(" data_buffer_error: %u", (uint32_t)qtd_token.data_buffer_error);
  klog_debug(" halted: %u", (uint32_t)qtd_token.halted);
  klog_debug(" active: %u", (uint32_t)qtd_token.active);
  klog_debug(" error_counter: %u", (uint32_t)qtd_token.error_counter);
  klog_debug(" data_toggle: %u", (uint32_t)qtd_token.data_toggle);
  klog_debug(" total_bytes_to_transfer: %u", (uint32_t)qtd_token.total_bytes_to_transfer);
}

static inline
void kdebug_log_qtd(const EHCI_QTD *qtd){
  klog_debug(" next_td: 0x%X", (uint64_t)qtd->next_td);
  klog_debug(" alt_next_td: 0x%X", (uint64_t)qtd->alt_next_td);
  uint64_t buffer_pointer = ((uint64_t)qtd->buffer_pointer_high[0] << 32) | qtd->buffer_pointer_low[0];
  klog_debug(" buffer_pointer: 0x%X", buffer_pointer);
  kdebug_log_qtd_token(qtd->qtd_token);
}

static inline
void kdebug_log_hc_status(const EHCI_Controller *hc){
  const EHCI_USB_Status_Register *usbsts = (const EHCI_USB_Status_Register *)&hc->op_regs->usb_status;
  klog_debug("usb_interrupt: %u", (uint32_t)usbsts->usb_interrupt); 
  klog_debug("usb_error_interrupt: %u", (uint32_t)usbsts->usb_error_interrupt);
  klog_debug("port_change_detect: %u", (uint32_t)usbsts->port_change_detect);
  klog_debug("frame_list_rollover: %u", (uint32_t)usbsts->frame_list_rollover);
  klog_debug("host_system_error: %u", (uint32_t)usbsts->host_system_error);
  klog_debug("interrupt_on_async_advance: %u", (uint32_t)usbsts->interrupt_on_async_advance);
  klog_debug("hc_halted: %u", (uint32_t)usbsts->hc_halted);
  klog_debug("reclamation: %u", (uint32_t)usbsts->reclamation);
  klog_debug("perodic_schedule_status: %u", (uint32_t)usbsts->perodic_schedule_status);
  klog_debug("asynch_schedule_status: %u", (uint32_t)usbsts->asynch_schedule_status);
}

//====================================================================================

static void ehci_init_qtd(EHCI_QTD *previous_td, EHCI_QTD *current_td, bool toggle, uint8_t transfer_type, uint16_t size, uintptr_t data_physical_address) {
  kassert(size < 32767);
  kassert(transfer_type < 3);
  static const uint32_t QTD_TRANSFER_SIZE_SHIFT = 16;
  static const uint32_t QTD_TRANSFER_SIZE_MASK = 0x7FFF;
  static const uint32_t QTD_ERROR_COUNTER_SHIFT = 10;
  static const uint32_t QTD_TRANSFER_TYPE_SHIFT = 8;
  static const uint32_t QTD_ACTIVE_BIT = 1 << 7;
  static const uint32_t QTD_POINTER_TERMINATE = 0b01;
  static const uint32_t QTD_DATA_TOGGLE_BIT = 1 << 31;

  if(previous_td != 0) {
    previous_td->next_td = (uint32_t)(uintptr_t)current_td;
    previous_td->alt_next_td = (uint32_t)(uintptr_t)current_td;
  }

  memset(current_td, 0x00, sizeof(EHCI_QTD));
  current_td->next_td = QTD_POINTER_TERMINATE;
  current_td->alt_next_td = QTD_POINTER_TERMINATE;
  current_td->qtd_token = QTD_ACTIVE_BIT;
  current_td->qtd_token |= (3 << QTD_ERROR_COUNTER_SHIFT);
  current_td->qtd_token |= transfer_type << QTD_TRANSFER_TYPE_SHIFT;
  current_td->qtd_token |= (size & QTD_TRANSFER_SIZE_MASK) << QTD_TRANSFER_SIZE_SHIFT;
  current_td->qtd_token |= (toggle ? QTD_DATA_TOGGLE_BIT : 0);

  //TODO(Torin 2016-09-20) Larger than one page transfers
  //TODO(Torin 2016-10-03) Do a check on the HC to make sure that the HC is capable 
  //of supporting 64bit pointers
  kassert(size < 4096);
  current_td->buffer_pointer_low[0] = (uint32_t)data_physical_address;
  current_td->buffer_pointer_high[0] = (uint32_t)(data_physical_address >> 32);
}

typedef enum {
  USB_Speed_FULL = 0b00,
  USB_Speed_LOW = 0b01,
  USB_Speed_HIGH = 0b10,
} USB_Speed;

static void ehci_initalize_qh(EHCI_Queue_Head *qh, uint8_t device_address, uint8_t endpoint_number, USB_Speed speed, uint16_t max_packet_length){
  static const uint32_t DATA_TOGGLE_CONTROL_BIT = 1 << 14;
  static const uint32_t HEAD_OF_RECLAMATION_LIST_BIT = 1 << 15;
  static const uint32_t HIGH_BANDWIDTH_PIPE_MULT_ONE = 0b01 << 30;
  kassert(device_address < 64);
  kassert(endpoint_number < 16);
  kassert(max_packet_length <= 1024);
  qh->endpoint_characteristics = device_address;
  qh->endpoint_characteristics |= HEAD_OF_RECLAMATION_LIST_BIT; //TODO(Torin) REMOVE THIS
  qh->endpoint_characteristics |= (endpoint_number << 8);
  qh->endpoint_characteristics |= (speed << 12);
  qh->endpoint_characteristics |= DATA_TOGGLE_CONTROL_BIT;
  qh->endpoint_characteristics |= max_packet_length << 16;
  qh->endpoint_capabilities = HIGH_BANDWIDTH_PIPE_MULT_ONE;
  qh->qtd_token = 0;
}

static int 
ehci_check_qh_status(EHCI_Queue_Head *qh){
  static const uint32_t STATUS_ERROR_MASK = 0b01111100;
  static const uint32_t STATUS_HALTED = 1 << 6;
  static const uint32_t STATUS_DATA_BUFFER_ERROR = 1 << 5;
  static const uint32_t STATUS_ACTIVE = 1 << 7;
  const uint32_t error_status = qh->qtd_token & STATUS_ERROR_MASK;
  if(error_status != 0) return -1;
  if(qh->qtd_token & STATUS_ACTIVE) return 0;
  return 1;
}

static int
ehci_check_port_status(uint32_t volatile *port_register){
  EHCI_Port *port = (EHCI_Port *)port_register;
  klog_debug(" connect_status_change: %u", (uint32_t)port->connect_status_change);
  klog_debug(" port_enabled_change: %u", (uint32_t)port->port_enabled_change);
  klog_debug(" over_current_change: %u", (uint32_t)port->over_current_change);
  klog_debug(" port_enabled: %u", (uint32_t)port->port_enabled);
  return 1;
}

static inline
void ehci_disable_asynch_schedule(EHCI_Controller *hc){
  static const uint32_t USBCMD_ASYNCH_SCHEDULE_ENABLE_BIT = 1 << 5;
  static const uint32_t USBSTS_ASYNCH_SCHEDULE_STATUS_BIT = 1 << 15;
  hc->op_regs->usb_command = hc->op_regs->usb_command & ~USBCMD_ASYNCH_SCHEDULE_ENABLE_BIT;
  wait_for_condition((hc->op_regs->usb_status & USBSTS_ASYNCH_SCHEDULE_STATUS_BIT) == 0);
}

static inline
void ehci_enable_asynch_schedule(EHCI_Controller *hc){
  static const uint32_t USBCMD_ASYNCH_SCHEDULE_ENABLE_BIT = 1 << 5;
  static const uint32_t USBSTS_ASYNCH_SCHEDULE_STATUS_BIT = 1 << 15;
  hc->op_regs->usb_command = hc->op_regs->usb_command | USBCMD_ASYNCH_SCHEDULE_ENABLE_BIT;
  wait_for_condition(hc->op_regs->usb_status & USBSTS_ASYNCH_SCHEDULE_STATUS_BIT);
}

#if 0
static inline
void kdebug_log_usb_request(USB_Device_Request *request){
  klog_debug("request_type: %u", (uint32_t)request.type);
  klog_debug("request_request: %u", (uint32_t)request.request);
  klog_debug("request_value: %u", (uint32_t)request.value);
  klog_debug("request_index: %u", (uint32_t)request.index);
}
#endif

int ehci_control_transfer(EHCI_Controller *hc, uint8_t device_address, USB_Device_Request *request, uintptr_t data_physical_address){
  static const uint8_t QTD_TOKEN_TYPE_OUT = 0b00;
  static const uint8_t QTD_TOKEN_TYPE_IN = 0b01;
  static const uint8_t QTD_TOKEN_TYPE_SETUP = 0b10;
  
  EHCI_QTD *setup_qtd = &hc->qtd_array[0];
  EHCI_QTD *data_qtd = &hc->qtd_array[1];
  EHCI_QTD *status_qtd = &hc->qtd_array[2];

  ehci_init_qtd(0, setup_qtd, 0, QTD_TOKEN_TYPE_SETUP, 8, (uintptr_t)request);
  ehci_init_qtd(setup_qtd, data_qtd, 1, QTD_TOKEN_TYPE_IN, request->length, data_physical_address);
  ehci_init_qtd(data_qtd, status_qtd, 1, QTD_TOKEN_TYPE_OUT, 0, 0);

  ehci_disable_asynch_schedule(hc);
  EHCI_Queue_Head *qh = &hc->asynch_qh;
  ehci_initalize_qh(qh, device_address, 0, USB_Speed_HIGH, 64);
  qh->next_td = (uint32_t)(uintptr_t)setup_qtd;
  qh->alt_next_td = (uint32_t)(uintptr_t)setup_qtd;
  ehci_enable_asynch_schedule(hc);
  pit_wait_milliseconds(2);
  wait_for_condition(ehci_check_qh_status(qh))
  ehci_disable_asynch_schedule(hc);
  int status = ehci_check_qh_status(qh);
  if(status == 1) {
    klog_debug("control transfer sucuess");
    return 1;
  } else {
    klog_debug("CONTROL TRANSFER ERROR");
    klog_debug("setup_qtd: ");
    kdebug_log_qtd(setup_qtd);
    klog_debug("data_qtd: ");
    kdebug_log_qtd(data_qtd);
    klog_debug("status_qtd: ");
    kdebug_log_qtd(status_qtd);
    klog_debug("usb_status");
    kdebug_log_hc_status(hc);
    return 0;
  }
}

static const uint16_t USB_LANGID_ENGLISH_USA = 0x0409;

typedef struct {
  uint8_t device_address;
  uint8_t vendor_string[126];
  uint8_t product_string[126];
  uint8_t vendor_string_length;
  uint8_t product_string_length;
} USB_Device;

static inline
void utf16_to_ascii(uint8_t *ascii, uint16_t *utf16, size_t utf16_length){
  size_t step_count = utf16_length / 2;
  for(size_t i = 0; i < step_count; i++){
    ascii[i] = utf16[i];
  } 
}

int ehci_initalize_device(EHCI_Controller *hc){
  static const uint8_t QTD_TOKEN_TYPE_OUT = 0b00;
  static const uint8_t QTD_TOKEN_TYPE_IN = 0b01;
  static const uint8_t QTD_TOKEN_TYPE_SETUP = 0b10;

  USB_Device _device = {};
  USB_Device *device = &_device;

  USB_Device_Descriptor device_descriptor = {};
  USB_Device_Request request = {}; 
  request.type = USB_REQUEST_TYPE_DEVICE_TO_HOST; 
  request.request = USB_REQUEST_GET_DESCRIPTOR;
  request.value_high = USB_DESCRIPTOR_TYPE_DEVICE;
  request.value_low = 0;
  request.index = 0;
  request.length = 64;
  if(ehci_control_transfer(hc, 0, &request, (uintptr_t)&device_descriptor) == 0){
    #if 0
    klog_debug("length: %u", (uint32_t)device_descriptor.descriptor_length);
    klog_debug("descriptor_type: %u", (uint32_t)device_descriptor.descriptor_type);
    klog_debug("device_class: 0x%X", (uint64_t)device_descriptor.device_class);
    klog_debug("max_packet_size: %u", (uint32_t)device_descriptor.max_packet_size);
    klog_debug("vendor_id: 0x%X", (uint64_t)device_descriptor.vendor_id);
    klog_debug("product_id: 0x%X", (uint64_t)device_descriptor.product_id);
    klog_debug("vendor_string: 0x%X", (uint64_t)device_descriptor.vendor_string);
    klog_debug("product_string: 0x%X", (uint64_t)device_descriptor.product_string);
    #endif
    return 0;
  }

#if 0
  klog_debug("length: %u", (uint32_t)device_descriptor.descriptor_length);
  klog_debug("descriptor_type: %u", (uint32_t)device_descriptor.descriptor_type);
  klog_debug("device_class: 0x%X", (uint64_t)device_descriptor.device_class);
  klog_debug("max_packet_size: %u", (uint32_t)device_descriptor.max_packet_size);
  klog_debug("vendor_id: 0x%X", (uint64_t)device_descriptor.vendor_id);
  klog_debug("product_id: 0x%X", (uint64_t)device_descriptor.product_id);
  klog_debug("vendor_string: 0x%X", (uint64_t)device_descriptor.vendor_string);
  klog_debug("product_string: 0x%X", (uint64_t)device_descriptor.product_string);
#endif

  {
    uint8_t string_descriptor_buffer[256] = {};
    USB_String_Descriptor *string_descriptor = (USB_String_Descriptor *)string_descriptor_buffer;
    USB_Device_Request string_request = {};
    string_request.type = USB_REQUEST_TYPE_DEVICE_TO_HOST;
    string_request.request = USB_REQUEST_GET_DESCRIPTOR;
    string_request.value_high = USB_DESCRIPTOR_TYPE_STRING;
    string_request.value_low = 0;
    string_request.index = 0;
    string_request.length = 8;

    ehci_control_transfer(hc, 0, &string_request, (uintptr_t)string_descriptor);
    klog_debug("string_descriptor_length: %u", string_descriptor->length);
    uint16_t *supported_language_list = (uint16_t *)(((uintptr_t)string_descriptor) + 2);
    size_t supported_language_count = string_descriptor->length - 2;
    bool device_supports_english = false;
    for(size_t i = 0; i < supported_language_count; i++){
      if(supported_language_list[i] == USB_LANGID_ENGLISH_USA){
        device_supports_english = true; 
        break;
      }
    }

    if(device_supports_english == false){
      klog_error("usb device does not support english!");
      return 0;
    }

    string_request.index = USB_LANGID_ENGLISH_USA;
    string_request.value_low = device_descriptor.vendor_string; 
    string_request.length = 64;
    memset(string_descriptor, 0x00, sizeof(USB_String_Descriptor));
    ehci_control_transfer(hc, 0, &string_request, (uintptr_t)string_descriptor);
    if(string_descriptor->length > 0){
      utf16_to_ascii(device->vendor_string, string_descriptor->string, string_descriptor->length - 2);
      device->vendor_string_length = (string_descriptor->length - 2) / 2;
    }
        
    string_request.value_low = device_descriptor.product_string;
    memset(string_descriptor, 0x00, sizeof(USB_String_Descriptor));
    ehci_control_transfer(hc, 0, &string_request, (uintptr_t)string_descriptor);
    if(string_descriptor->length > 0){
      utf16_to_ascii(device->product_string, string_descriptor->string, string_descriptor->length - 2);
      device->product_string_length = (string_descriptor->length - 2) / 2;
    }
  }

  klog_debug("usb_device_info:");
  klog_debug(" vendor_string: %.*s", device->vendor_string_length, device->vendor_string);
  klog_debug(" product_string: %.*s", device->product_string_length, device->product_string);

  USB_Device_Request set_address_request = {};
  set_address_request.type = 0;
  set_address_request.request = USB_REQUEST_SET_ADDRESSS;
  set_address_request.value_low = 1;
  set_address_request.index = 0;
  set_address_request.length = 0;
  EHCI_QTD *setup_qtd = &hc->qtd_array[0];
  EHCI_QTD *status_qtd = &hc->qtd_array[1];
  ehci_init_qtd(0, setup_qtd, 0, QTD_TOKEN_TYPE_SETUP, sizeof(USB_Device_Request), (uintptr_t)&set_address_request);
  ehci_init_qtd(setup_qtd, status_qtd, 1, QTD_TOKEN_TYPE_IN, 0, 0);
  ehci_disable_asynch_schedule(hc);
  EHCI_Queue_Head *qh = &hc->asynch_qh;
  ehci_initalize_qh(qh, 0, 0, USB_Speed_HIGH, 64);
  qh->next_td = (uint32_t)(uintptr_t)setup_qtd;
  qh->alt_next_td = (uint32_t)(uintptr_t)setup_qtd;
  ehci_enable_asynch_schedule(hc);
  pit_wait_milliseconds(2);
  wait_for_condition(ehci_check_qh_status(qh))
  ehci_disable_asynch_schedule(hc);
  int status = ehci_check_qh_status(qh);
  if(status == 1) {
    klog_debug("control transfer sucuess");
  } else {
    klog_debug("CONTROL TRANSFER ERROR");
    klog_debug(" ");
    klog_debug("setup_qtd: ");
    kdebug_log_qtd(setup_qtd);
    klog_debug(" ");
    klog_debug("status_qtd: ");
    kdebug_log_qtd(status_qtd);
    klog_debug("usb_status");
    kdebug_log_hc_status(hc);
    return 0;
  }

  return 1;
}

static bool 
ehci_reset_port(volatile uint32_t *port){
  static const uint32_t EHCI_PORT_RESET_BIT = 1 << 8;
  static const uint32_t EHCI_PORT_ENABLED_BIT = 1 << 2;
  static const uint32_t EHCI_PORT_ENABLED_CHANGED_BIT = 1 << 3;

  *port = *port | EHCI_PORT_RESET_BIT;
  kassert(*port & EHCI_PORT_RESET_BIT);
  //NOTE(Torin 2016-09-17) Must Wait Max 50ms (USB2.0 spec 10.2.8.1)
  pit_wait_milliseconds(50);
  klog_debug("waited 50ms");
  *port = *port & ~EHCI_PORT_RESET_BIT;
  //NOTE(Torin 2016-09-25) The HC has 2ms to set RESET bit to 0
  wait_for_condition((*port & EHCI_PORT_RESET_BIT) == 0)
  kassert((*port & EHCI_PORT_RESET_BIT) == 0);
  klog_debug("port was reset");

  if(*port & EHCI_PORT_ENABLED_BIT) {
    klog_debug("port was enabled");
    return true;
  }

  if(*port & EHCI_PORT_ENABLED_CHANGED_BIT){
    *port = *port & EHCI_PORT_ENABLED_CHANGED_BIT;
    if(*port & EHCI_PORT_ENABLED_BIT) {
      klog_debug("port was enabled with the changed bit set?");
      return true;
    }
  }

  return false;
}

static inline
int ehci_is_hc_halted(EHCI_Operational_Registers *op_regs){
  static const uint32_t USBSTATUS_CONTROLLER_HALTED = (1 << 12);
  int result = op_regs->usb_status & USBSTATUS_CONTROLLER_HALTED;
  return result;
}

int ehci_initalize(uintptr_t ehci_physical_address, PCI_Device *pci_device){
  klog_debug("starting ehci initalization...");

  //TODO(Torin 2016-09-16) Idealy this memory_map should occur in a different location
  uintptr_t ehci_virtual_page = 0x1000000;  
  uintptr_t page_offset = kmem_map_unaligned_physical_to_aligned_virtual_2MB(ehci_physical_address, ehci_virtual_page, 0); 
  uintptr_t ehci_virtual_address = page_offset + ehci_virtual_page;

  bool use_64_bit_ds = false;

  EHCI_Controller *hc = &g_ehci;
  hc->pci_device = *pci_device;
  EHCI_Capability_Registers *cap_regs = (EHCI_Capability_Registers *)ehci_virtual_address;
  uintptr_t operational_register_base = ehci_virtual_address + cap_regs->capability_length;
  EHCI_Operational_Registers *op_regs = (EHCI_Operational_Registers *)operational_register_base;
  hc->op_regs = op_regs;
  
  { //NOTE(Torin 2016-09-04) Extract information from hhcparams register
    static const uint32_t HHCPARAMS_EXT_CAPS_MASK = 0xFF00;
    static const uint32_t HHCPARAMS_ADDRESSING_BIT = (1 << 0);
    static const uint32_t ADDRESSING_CAPABILITY_32 = 0b00;
    static const uint32_t ADDRESSING_CAPABILITY_64 = 0b01;
    uint32_t addressing_capability = cap_regs->hcc_params & HHCPARAMS_ADDRESSING_BIT;
    uint32_t extended_capabilities = (cap_regs->hcc_params & HHCPARAMS_EXT_CAPS_MASK) >> 8;
    if(addressing_capability == ADDRESSING_CAPABILITY_64) use_64_bit_ds = true;
    if(extended_capabilities >= 0x40){
      //TODO(Torin 2016-09-25) Need better error handling to bail out if taking 
      //ownership of the EHCI controler here fails for some reason
      static const uint32_t EHCI_USBLEGSUP_REGISTER_OFFSET = 0x00;
      uint32_t legacy_support_register = extended_capabilities + EHCI_USBLEGSUP_REGISTER_OFFSET;
      pci_set_config_address(pci_device->bus_number, pci_device->device_number, 
        pci_device->function_number, legacy_support_register);
      uint32_t legacy_support = pci_read_uint32();
      static const uint32_t USBLEGSUP_OS_OWNERSHIP = 1 << 24;
      static const uint32_t USBLEGSUP_BIOS_OWNERSHIP = 1 << 16;
      if(legacy_support & USBLEGSUP_BIOS_OWNERSHIP){
        volatile uint32_t legacy_with_os_ownership = legacy_support | USBLEGSUP_OS_OWNERSHIP;
        pci_write_uint32(legacy_with_os_ownership);
        wait_for_condition(pci_read_uint32() & USBLEGSUP_OS_OWNERSHIP)
        wait_for_condition((pci_read_uint32() & USBLEGSUP_BIOS_OWNERSHIP) == 0)
      }
    }
  }

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
    //NOTE(Torin 2016-10-04) Setup Peroidic Queue Head
    periodic_qh->horizontal_link_pointer = EHCI_POINTER_TERMINATE;
    periodic_qh->current_td = EHCI_POINTER_TERMINATE;
    periodic_qh->next_td = EHCI_POINTER_TERMINATE;
    periodic_qh->alt_next_td = EHCI_POINTER_TERMINATE;
    //NOTE(Torin 2016-10-04) Setup perodic framelist 
    uint32_t *periodic_frame_list = g_ehci.periodic_frame_list;
    for(size_t i = 0; i < 1024; i++){
      periodic_frame_list[i] = (uint32_t)(uintptr_t)periodic_qh | EHCI_POINTER_TYPE_QH | EHCI_POINTER_TERMINATE;
    }
  }

  { //NOTE(Torin 2016-09-13) Set nessecary registers and initalize the controller 
    static const uint32_t USBCMD_RUN_STOP = (1 << 0);
    static const uint32_t USBCMD_HCRESET = 1 << 1;
    static const uint32_t USBCMD_PERIODIC_SCHEDULE_ENABLE = (1 << 4);
    static const uint32_t USBCMD_ASYNCH_SCHEDULE_ENABLE = (1 << 5);
    static const uint32_t USBCMD_INTERRUPT_THRESHOLD_CONTROL_8 = (0x08 << 16); //1ms

    static const uint32_t USBSTATUS_CONTROLLER_HALTED = (1 << 12);
    static const uint32_t USBSTATUS_HOST_SYSTEM_ERROR = 1 << 4;
    //NOTE(Torin 2016-09-04) Set all devices to be managed by this EHCI 
    static const uint32_t ROUTE_PORTS_TO_IMPLEMENTATION_DEPENDENT_CONTROLLER = 0b00;
    static const uint32_t ROUTE_PORTS_TO_HOST_EHCI = 0b01;

    op_regs->usb_command = op_regs->usb_command & (~USBCMD_RUN_STOP);
    wait_for_condition((op_regs->usb_command & USBCMD_RUN_STOP) == 0);
    op_regs->usb_command = op_regs->usb_command | USBCMD_HCRESET;
    wait_for_condition((op_regs->usb_command & USBCMD_HCRESET) == 0);

    op_regs->ctrl_ds_segment = 0;
    op_regs->usb_interrupt = 0;
    op_regs->ctrl_ds_segment = 0x00;
    op_regs->perodic_list_base = (uint32_t)(uintptr_t)g_ehci.periodic_frame_list;
    op_regs->async_list_address = (uint32_t)(uintptr_t)&g_ehci.asynch_qh;
    op_regs->frame_index = 0;
    op_regs->config_flag = ROUTE_PORTS_TO_HOST_EHCI;
    op_regs->usb_command = USBCMD_INTERRUPT_THRESHOLD_CONTROL_8 | 
      USBCMD_ASYNCH_SCHEDULE_ENABLE | USBCMD_RUN_STOP; 
    wait_for_condition((op_regs->usb_status & USBSTATUS_CONTROLLER_HALTED) == 0);
    klog_debug("ehci controller was started");
 
    if(op_regs->usb_status & USBSTATUS_CONTROLLER_HALTED){
      klog_debug("CONTROLLER UNEXPECTEDLY HALTED");
      if(op_regs->usb_status & USBSTATUS_HOST_SYSTEM_ERROR) {
        klog_debug("HOST SYSTEM ERROR");
      } 
    }
    op_regs->config_flag = ROUTE_PORTS_TO_HOST_EHCI;
  }

  static const uint32_t USBSTATUS_CONTROLLER_HALTED = (1 << 12);


  size_t port_count = 0;
  bool is_port_power_control_enabled = false;
  { //NOTE(Torin 2016-09-17) Extract HCSPARAMS information
    static const uint32_t HCS_PORT_COUNT_MASK = 0b1111;
    static const uint32_t POWER_PORT_CONTROL_BIT = 1 << 4;
    port_count = cap_regs->hcs_params & HCS_PORT_COUNT_MASK; 
    is_port_power_control_enabled = cap_regs->hcs_params & POWER_PORT_CONTROL_BIT;
  }

  klog_debug("is_port_power_control_enabled: %u", (uint32_t)is_port_power_control_enabled);




  { //NOTE(Torin 2016-09-06) Probe the ports managed by this controler
    klog_debug("enumerating %lu ports", port_count);
    for(size_t i = 0; i < port_count; i++){
      kassert((op_regs->usb_status & USBSTATUS_CONTROLLER_HALTED) == 0);
      static const uint32_t PORT_ENABLED_BIT = (1 << 2);
      static const uint32_t PORT_POWER_BIT = 1 << 12;

      static const uint32_t PORT_ENABLED_STATUS_CHANGED_BIT = 1 << 3;
      static const uint32_t PORT_CONNECT_STATUS_CHANGED_BIT = 1 << 1;
      static const uint32_t PORT_ENALBED_STATUS_BIT = 1 << 2;
      static const uint32_t PORT_CONNECT_STATUS_BIT = 1 << 0;

      static const uint32_t PORT_LINE_STATUS_LOW_SPEED_DEVICE = 1 << 10;

      uint32_t volatile *port_reg = &op_regs->ports[i];
      if(*port_reg & PORT_CONNECT_STATUS_CHANGED_BIT){
        *port_reg = *port_reg | PORT_CONNECT_STATUS_CHANGED_BIT; //clears out connect status change
        wait_for_condition((*port_reg & PORT_CONNECT_STATUS_CHANGED_BIT) == 0);
        if(*port_reg & PORT_CONNECT_STATUS_BIT){
          pit_wait_milliseconds(100);
          klog_debug("USB_Device connected on port %lu", i);
          bool is_port_enabled = *port_reg & PORT_ENABLED_BIT;
          if(is_port_enabled == false) {
            if(*port_reg & PORT_LINE_STATUS_LOW_SPEED_DEVICE){
              klog_error("port is a low speed device!");
            }
            kassert(ehci_is_hc_halted(op_regs) == 0);
            is_port_enabled = ehci_reset_port(port_reg);
          }
          if(is_port_enabled == false){
            klog_debug("Port %lu failed to enable", i);
          } else {
            klog_debug("Port %lu enabled sucuessfuly", i);
            pit_wait_milliseconds(10);
            ehci_initalize_device(&g_ehci);
          }
        }
      }
      klog_debug("no device connected on port %lu", i);
    }
  }
  klog_debug("echi was initalized");
  return 1;
}
