#define wait_for_condition(x, timeout) { \
  globals.lapic_timer_ticks = 0; \
  while((!(x)) && (globals.lapic_timer_ticks < timeout)) { asm volatile("nop"); } \
  if(globals.lapic_timer_ticks >= timeout) { klog_debug("wait timed out: %s,  %s:%u", #x, __FILE__, __LINE__); }} \
  if(globals.lapic_timer_ticks >= timeout)

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

static_assert(sizeof(EHCI_Port) == 4);

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

static void ehci_initalize_qh(EHCI_Queue_Head *qh, EHCI_QTD *qtd, uint8_t device_address, uint8_t endpoint_number, USB_Speed speed, uint16_t max_packet_length){
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
  qh->next_td = (uint32_t)(uintptr_t)qtd;
  qh->alt_next_td = (uint32_t)(uintptr_t)qtd;
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
  if(((qh->qtd_token & STATUS_ACTIVE) == 0) && qh->next_td == 1 && qh->alt_next_td == 1) {
    //NOTE(Torin 2016-10-16) Make sure the transaction is actualy done and not in a rare
    //intermediant case while the EHCI Controler is in the middle of writing data into the overlay
    asm volatile("nop");
    if(((qh->qtd_token & STATUS_ACTIVE) == 0) && qh->next_td == 1 && qh->alt_next_td == 1) {
      return 1;
    }
  }
  return 0;
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
  wait_for_condition((hc->op_regs->usb_status & USBSTS_ASYNCH_SCHEDULE_STATUS_BIT) == 0, 2){
    klog_error("asynchrous shedule failed to disable");
  }
}

static inline
void ehci_enable_asynch_schedule(EHCI_Controller *hc){
  static const uint32_t USBCMD_ASYNCH_SCHEDULE_ENABLE_BIT = 1 << 5;
  static const uint32_t USBSTS_ASYNCH_SCHEDULE_STATUS_BIT = 1 << 15;
  hc->op_regs->usb_command = hc->op_regs->usb_command | USBCMD_ASYNCH_SCHEDULE_ENABLE_BIT;
  wait_for_condition(hc->op_regs->usb_status & USBSTS_ASYNCH_SCHEDULE_STATUS_BIT, 2){
    klog_error("failed to enable asynchronous schedule");
  }
}

//==========================================================================================================

 int ehci_control_transfer_without_data(EHCI_Controller *hc, uint8_t device_address, USB_Device_Request *request){
  static const uint8_t QTD_TOKEN_TYPE_OUT = 0b00;
  static const uint8_t QTD_TOKEN_TYPE_IN = 0b01;
  static const uint8_t QTD_TOKEN_TYPE_SETUP = 0b10;
  EHCI_QTD *setup_qtd = &hc->qtd_array[0];
  EHCI_QTD *status_qtd = &hc->qtd_array[1];
  ehci_init_qtd(0, setup_qtd, 0, QTD_TOKEN_TYPE_SETUP, sizeof(USB_Device_Request), (uintptr_t)request);
  ehci_init_qtd(setup_qtd, status_qtd, 1, QTD_TOKEN_TYPE_IN, 0, 0);

  ehci_disable_asynch_schedule(hc);
  EHCI_Queue_Head *qh = &hc->asynch_qh;
  ehci_initalize_qh(qh, setup_qtd, device_address, 0, USB_Speed_HIGH, 64);
  ehci_enable_asynch_schedule(hc);
  wait_for_condition(ehci_check_qh_status(qh), 125) {}
  ehci_disable_asynch_schedule(hc);

  int status = ehci_check_qh_status(qh);
  if(status == 0){
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

//TODO(Torin 2016-10-16) Find out the max timeout value for a usb transaction
//to wait for the qtd to become inactive

static inline
int ehci_internal_control_transfer_with_data(EHCI_Controller *hc, EHCI_Queue_Head *qh, EHCI_QTD *setup_qtd, EHCI_QTD *data_qtd, EHCI_QTD *status_qtd, 
uint8_t device_address, USB_Device_Request *request, uintptr_t data_physical_address){
  static const uint8_t QTD_TOKEN_TYPE_OUT = 0b00;
  static const uint8_t QTD_TOKEN_TYPE_IN = 0b01;
  static const uint8_t QTD_TOKEN_TYPE_SETUP = 0b10;
  ehci_init_qtd(0, setup_qtd, 0, QTD_TOKEN_TYPE_SETUP, 8, (uintptr_t)request);
  ehci_init_qtd(setup_qtd, data_qtd, 1, request->direction ? QTD_TOKEN_TYPE_IN : QTD_TOKEN_TYPE_OUT, request->length, data_physical_address);
  ehci_init_qtd(data_qtd, status_qtd, 1, request->direction ? QTD_TOKEN_TYPE_OUT : QTD_TOKEN_TYPE_IN, 0, 0);
  ehci_disable_asynch_schedule(hc);
  ehci_initalize_qh(qh, setup_qtd, device_address, 0, USB_Speed_HIGH, 64);
  ehci_enable_asynch_schedule(hc);
  wait_for_condition(ehci_check_qh_status(qh), 125){}
  ehci_disable_asynch_schedule(hc);
  int status = ehci_check_qh_status(qh);
  return status;
}

static inline 
int ehci_control_transfer_with_data(EHCI_Controller *hc, uint8_t device_address, USB_Device_Request *request, uintptr_t data_physical_address){
  EHCI_QTD *setup_qtd = &hc->qtd_array[0];
  EHCI_QTD *data_qtd = &hc->qtd_array[1];
  EHCI_QTD *status_qtd = &hc->qtd_array[2];
  EHCI_Queue_Head *qh = &hc->asynch_qh;
  int status = ehci_internal_control_transfer_with_data(hc, qh, setup_qtd, data_qtd, status_qtd, 
    device_address, request, data_physical_address);
  if(status == -1){
    klog_error("[CONTROL_TRANSFER_ERROR]");
    klog_debug("qh_qtd:");
    kdebug_log_qtd_token(qh->qtd_token);
    klog_debug("setup_qtd: ");
    kdebug_log_qtd(setup_qtd);
    klog_debug("data_qtd: ");
    kdebug_log_qtd(data_qtd);
    klog_debug("status_qtd: ");
    kdebug_log_qtd(status_qtd);
    klog_debug("usb_status");
    kdebug_log_hc_status(hc);
    return 0;
  } else if (status == 0){
    klog_warning("failed first control transfer");
    status = ehci_internal_control_transfer_with_data(hc, qh, setup_qtd, data_qtd, status_qtd, 
      device_address, request, data_physical_address);
    if(status == 0){
      klog_error("[CONTROL_TRANSFER_TIMEOUT");
      klog_debug("qh_qtd:");
      kdebug_log_qtd_token(qh->qtd_token);
      klog_debug("setup_qtd: ");
      kdebug_log_qtd(setup_qtd);
      klog_debug("data_qtd: ");
      kdebug_log_qtd(data_qtd);
      klog_debug("status_qtd: ");
      kdebug_log_qtd(status_qtd);
      klog_debug("usb_status");
      kdebug_log_hc_status(hc);
      return 0;
    } else if (status == -1){
      klog_error("[CONTROL_TRANSFER_ERROR]");
      klog_debug("qh_qtd:");
      kdebug_log_qtd_token(qh->qtd_token);
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
  return 1;
}

int ehci_bulk_transfer_with_data(EHCI_Controller *hc, USB_Mass_Storage_Device *msd, USB_Command_Block_Wrapper *cbw, size_t out_length, void *in_data, size_t in_length){
  static const uint8_t QTD_TOKEN_TYPE_OUT = 0b00;
  static const uint8_t QTD_TOKEN_TYPE_IN = 0b01;
  //NOTE(Torin 2016-10-06) This is set to failed to catch errors if the tranaction never transfers any data
  static const uint8_t CSW_COMMAND_PASSED = 0x00;
  static const uint8_t CSW_COMMAND_FAILED = 0x01;
  static const uint8_t CSW_STATUS_INVALID = 0xFF; //NOTE(Torin) Valued used by kernel to determine if status was not recieved correctly
  USB_Command_Status_Wrapper csw = {};
  csw.status = CSW_COMMAND_FAILED;
  //Preallocate Required data strucutres
  EHCI_QTD *out_data_qtd = &hc->qtd_array[0];
  EHCI_QTD *in_data_qtd = &hc->qtd_array[1];
  EHCI_QTD *in_status_qtd = &hc->qtd_array[2];
  ehci_init_qtd(0, out_data_qtd, msd->out_toggle_value, QTD_TOKEN_TYPE_OUT, out_length, (uintptr_t)cbw);
  ehci_init_qtd(0, in_data_qtd, msd->in_toggle_value, QTD_TOKEN_TYPE_IN, in_length, (uintptr_t)in_data);
  ehci_init_qtd(in_data_qtd, in_status_qtd, !msd->in_toggle_value, QTD_TOKEN_TYPE_IN, sizeof(USB_Command_Status_Wrapper), (uintptr_t)&csw);
  msd->out_toggle_value = !msd->out_toggle_value;

  EHCI_Queue_Head *out_qh = &hc->asynch_qh;
  ehci_disable_asynch_schedule(hc);
  ehci_initalize_qh(out_qh, out_data_qtd, msd->device_number, msd->out_endpoint, USB_Speed_HIGH, msd->out_endpoint_max_packet_size);
  ehci_enable_asynch_schedule(hc);
  wait_for_condition(ehci_check_qh_status(out_qh), 125){}
  ehci_disable_asynch_schedule(hc);
  int status = ehci_check_qh_status(out_qh);
  if(status == 0) {
    klog_error("bulk out transaction error");
    klog_debug("queue_head:");
    kdebug_log_qtd_token(out_qh->qtd_token);
    kdebug_log_qtd(out_data_qtd);
    return 0;
  } 

  EHCI_Queue_Head *in_qh = &hc->asynch_qh;
  ehci_disable_asynch_schedule(hc);
  ehci_initalize_qh(in_qh, in_data_qtd, msd->device_number, msd->in_endpoint, USB_Speed_HIGH, msd->in_endpoint_max_packet_size);
  ehci_enable_asynch_schedule(hc);
  wait_for_condition(ehci_check_qh_status(in_qh), 1000) {}
  ehci_disable_asynch_schedule(hc);
  status = ehci_check_qh_status(in_qh);

  if(status == 0) {
    klog_error("bulk in transaction timed out without completing");
    klog_debug("queue_head:");
    kdebug_log_qtd_token(in_qh->qtd_token);
    klog_debug("in_data:");
    kdebug_log_qtd(in_data_qtd);
    klog_debug("in_status:");
    kdebug_log_qtd(in_status_qtd);
    return 0;
  } else if (status == -1) {
    klog_error("bulk in tranaction resulted in an error");
    klog_debug("queue_head:");
    kdebug_log_qtd_token(in_qh->qtd_token);
    klog_debug("in_data:");
    kdebug_log_qtd(in_data_qtd);
    klog_debug("in_status:");
    kdebug_log_qtd(in_status_qtd);
    return 0;
  }

  if(csw.status == CSW_STATUS_INVALID){
    klog_warning("wtf mate STATUS is invalid???");
  }

  if(csw.status == CSW_COMMAND_FAILED){
    klog_error("usb bulk transaction reports failure");
    return 0;
  } else {
    if(csw.signature != USB_CSW_SIGNATURE) {
      klog_error("malformed command status wrapper signature!");
      return 0;
    }
    if(csw.tag != cbw->tag){
      klog_error("command wrapper tag mistmatch!");
      return 0;
    }
  }

  return 1;
}


int ehci_bulk_transfer_no_data(EHCI_Controller *hc, USB_Mass_Storage_Device *msd, USB_Command_Block_Wrapper *cbw){
  static const uint8_t QTD_TOKEN_TYPE_OUT = 0b00;
  static const uint8_t QTD_TOKEN_TYPE_IN = 0b01;
  //NOTE(Torin 2016-10-06) This is set to failed to catch errors if the tranaction never transfers any data
  static const uint8_t CSW_COMMAND_PASSED = 0x00;
  static const uint8_t CSW_COMMAND_FAILED = 0x01;
  USB_Command_Status_Wrapper csw = {};
  csw.status = CSW_COMMAND_FAILED;
  //Preallocate Required data strucutres
  EHCI_QTD *out_data_qtd = &hc->qtd_array[0];
  EHCI_QTD *in_status_qtd = &hc->qtd_array[1];
  ehci_init_qtd(0, out_data_qtd, msd->out_toggle_value, QTD_TOKEN_TYPE_OUT, 31, (uintptr_t)cbw);
  ehci_init_qtd(0, in_status_qtd, msd->in_toggle_value, QTD_TOKEN_TYPE_IN, sizeof(USB_Command_Status_Wrapper), (uintptr_t)&csw);
  msd->out_toggle_value = !msd->out_toggle_value;
  msd->in_toggle_value = !msd->in_toggle_value;

  EHCI_Queue_Head *out_qh = &hc->asynch_qh;
  ehci_disable_asynch_schedule(hc);
  ehci_initalize_qh(out_qh, out_data_qtd, msd->device_number, msd->out_endpoint, USB_Speed_HIGH, msd->out_endpoint_max_packet_size);
  ehci_enable_asynch_schedule(hc);
  wait_for_condition(ehci_check_qh_status(out_qh), 125){}
  ehci_disable_asynch_schedule(hc);
  int status = ehci_check_qh_status(out_qh);
  if(status == 0) {
    klog_error("bulk out transaction error");
    klog_debug("queue_head:");
    kdebug_log_qtd_token(out_qh->qtd_token);
    kdebug_log_qtd(out_data_qtd);
    return 0;
  } 

  EHCI_Queue_Head *in_qh = &hc->asynch_qh;
  ehci_disable_asynch_schedule(hc);
  ehci_initalize_qh(in_qh, in_status_qtd, msd->device_number, msd->in_endpoint, USB_Speed_HIGH, msd->in_endpoint_max_packet_size);
  ehci_enable_asynch_schedule(hc);
  wait_for_condition(ehci_check_qh_status(in_qh), 125){}
  status = ehci_check_qh_status(in_qh);
  ehci_disable_asynch_schedule(hc);
  if(status == 0) {
    klog_error("bulk in transaction timed out without completing");
    klog_debug("queue_head:");
    kdebug_log_qtd_token(in_qh->qtd_token);
    klog_debug("in_status:");
    kdebug_log_qtd(in_status_qtd);
    return 0;
  } else if (status == -1) {
    klog_error("bulk in tranaction resulted in an error");
    klog_debug("queue_head:");
    kdebug_log_qtd_token(in_qh->qtd_token);
    klog_debug("in_status:");
    kdebug_log_qtd(in_status_qtd);
    return 0;
  }

  if(csw.status == CSW_COMMAND_FAILED){
    klog_error("usb bulk transaction reports failure");
    return 0;
  } else {
    if(csw.signature != USB_CSW_SIGNATURE) {
      klog_error("malformed command status wrapper signature!");
      return 0;
    }
    if(csw.tag != cbw->tag){
      klog_error("command wrapper tag mistmatch!");
      return 0;
    }
  }

  return 1;
}

//=====================================================================
static inline
void utf16_to_ascii(uint8_t *ascii, uint16_t *utf16, size_t utf16_length){
  size_t step_count = utf16_length / 2;
  for(size_t i = 0; i < step_count; i++){
    ascii[i] = utf16[i];
  } 
}

static inline
int ehci_get_descriptor(EHCI_Controller *hc, uint8_t descriptor_type, uint8_t descriptor_index, uint16_t langid, uint16_t length, void *data){
  USB_Device_Request request = {};
  request.direction = 1;
  request.request = USB_REQUEST_GET_DESCRIPTOR;
  request.value_low = descriptor_index;
  request.value_high = descriptor_type;
  request.index_high = (uint8_t)(langid >> 8);
  request.index_low = (uint8_t)langid;
  request.length = length;
  int result = ehci_control_transfer_with_data(hc, 0, &request, (uintptr_t)data);
  return result;
}



#include "filesystem.c"

static int ehci_read_to_physical_address(EHCI_Controller *hc, USB_Mass_Storage_Device *msd, uintptr_t out_data, uint32_t start_block, uint16_t block_count){
  if(msd->logical_block_count < (start_block + block_count)){
    klog_error("usb read excedes device maximums");
    return 0;
  }

  SCSI_Read_Command read_command = {
    .cbw.signature = USB_CBW_SIGNATURE,
    .cbw.tag = 0x28,
    .cbw.transfer_length = msd->logical_block_size, 
    .cbw.direction = 1, //device-to-host
    .cbw.length = 10,
    .operation_code = 0x28,
    .logical_block_address_3 = start_block >> 24,
    .logical_block_address_2 = start_block >> 16,
    .logical_block_address_1 = start_block >> 8,
    .logical_block_address_0 = start_block >> 0,
    .transfer_length_1 = block_count >> 8,
    .transfer_length_0 = block_count >> 0,
  };

  if(ehci_bulk_transfer_with_data(hc, msd, &read_command.cbw, 31, (void *)out_data, msd->logical_block_size * block_count) == 0){
    uint32_t logical_block_address = read_command.logical_block_address_0;
    logical_block_address |= read_command.logical_block_address_1 << 8;
    logical_block_address |= read_command.logical_block_address_2 << 16;
    logical_block_address |= read_command.logical_block_address_3 << 24;
    uint16_t transfer_length = read_command.transfer_length_0; 
    transfer_length |= read_command.transfer_length_1 << 8;

    klog_error("[FAILED] USB MSD Read");
    klog_debug(" cbw.transfer_length: %u", (uint32_t)read_command.cbw.transfer_length);
    klog_debug(" transfer_length: %u", (uint32_t)transfer_length);
    klog_debug(" logical_block_address: %u", (uint32_t)logical_block_address);
    return 0;
  } 

  return 1;
}


static Ext2_Filesystem g_ext2;

int ehci_initalize_device(EHCI_Controller *hc, USB_Device *device){
  USB_Device_Descriptor device_descriptor = {};
  if(ehci_get_descriptor(hc, USB_DESCRIPTOR_TYPE_DEVICE, 0, 0, 64, &device_descriptor) == 0){
    klog_debug("failed to get device descriptor");
    return 0;
  }

#if 0
  USB_Device_Request request = {};
  request.direction = 1; 
  request.request = USB_REQUEST_GET_DESCRIPTOR;
  request.value_high = USB_DESCRIPTOR_TYPE_DEVICE;
  request.value_low = 0;
  request.length = 64;
  if(ehci_control_transfer_with_data(hc, 0, false, &request, (uintptr_t)&device_descriptor) == 0){
    klog_error("get device descriptor failed  for device");
    return 0;
  }
  kdebug_log_usb_descriptor(device_descriptor);
#endif

  device->vendor_id = device_descriptor.vendor_id;
  device->product_id = device_descriptor.product_id;
  device->device_class = device_descriptor.device_class;

  {
    uint8_t string_descriptor_buffer[256] = {};
    if(ehci_get_descriptor(hc, USB_DESCRIPTOR_TYPE_STRING, 0, 0, 8, string_descriptor_buffer) == 0){
      klog_debug("failed to get string descriptor list");
      return 0;
    }

#if 0
    string_request.type = USB_REQUEST_TYPE_DEVICE_TO_HOST;
    string_request.request = USB_REQUEST_GET_DESCRIPTOR;
    string_request.value_high = USB_DESCRIPTOR_TYPE_STRING;
    string_request.value_low = 0; //NOTE(Torin) Requesting Language Information
    string_request.length = 8;
    ehci_control_transfer_with_data(hc, 0, false, &string_request, (uintptr_t)string_descriptor);
  #endif 

    USB_String_Descriptor *string_descriptor = (USB_String_Descriptor *)string_descriptor_buffer;
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

    //TODO(Torin 2016-10-09) Support non-english languages
    if(device_supports_english == false){
      klog_error("usb device does not support english!");
      return 0;
    }

#if 0
    string_request.index = USB_LANGID_ENGLISH_USA;
    string_request.value_low = device_descriptor.vendor_string; 
    string_request.length = 64;
    memset(string_descriptor, 0x00, sizeof(USB_String_Descriptor));
#endif

    ehci_get_descriptor(hc, USB_DESCRIPTOR_TYPE_STRING, device_descriptor.vendor_string, USB_LANGID_ENGLISH_USA, 64, string_descriptor_buffer);


    if(string_descriptor->length > 0){
      utf16_to_ascii(device->vendor_string, string_descriptor->string, string_descriptor->length - 2);
      device->vendor_string_length = (string_descriptor->length - 2) / 2;
    }
        
    ehci_get_descriptor(hc, USB_DESCRIPTOR_TYPE_STRING, device_descriptor.product_string, USB_LANGID_ENGLISH_USA, 64, string_descriptor_buffer);
    if(string_descriptor->length > 0){
      utf16_to_ascii(device->product_string, string_descriptor->string, string_descriptor->length - 2);
      device->product_string_length = (string_descriptor->length - 2) / 2;
    }
  }
  USB_Mass_Storage_Device _msd = {};
  USB_Mass_Storage_Device *msd = &_msd;

  uint8_t configuration_value = 0;
  uint8_t configuration_buffer[256] = {};
  klog_debug("configuration_count: %u", (uint32_t)device_descriptor.config_count);


  for(size_t config_index = 0; config_index < device_descriptor.config_count; config_index++){
    klog_debug("config_index: %lu", config_index);
    USB_Configuration_Descriptor *config = (USB_Configuration_Descriptor *)configuration_buffer;
    ehci_get_descriptor(hc, USB_DESCRIPTOR_TYPE_CONFIG, config_index, 0,  64, configuration_buffer);
    //TODO(Torin) Have an entire page mapped for this buffer
    //Hell just make it per kernel thread... Problematic for interrupts?
    if(config->total_length > sizeof(configuration_buffer)){
      klog_error("configuration descriptor is too large for provided buffer");
      return 0;
    }

    USB_Interface_Descriptor *interface = (USB_Interface_Descriptor *)(config + 1);
    for(size_t i = 0; i < config->interface_count; i++){
      static const uint8_t SUBCLASS_SCSI = 0x06;
      USB_Endpoint_Descriptor *endpoint = (USB_Endpoint_Descriptor *)(interface + 1);
      if(interface->interface_class == USB_DEVICE_CLASS_MASS_STORAGE && 
          interface->interface_subclass == SUBCLASS_SCSI && 
          interface->interface_protocol == USB_MASS_STORAGE_PROTOCOL_BULK_ONLY) {
        for(size_t j = 0; j < interface->endpoint_count; j++){
          if(endpoint->endpoint_direction == 0){
            msd->out_endpoint = endpoint->endpoint_number;
            msd->out_endpoint_max_packet_size = endpoint->max_packet_size;
          } else if (endpoint->endpoint_direction == 1){
            msd->in_endpoint = endpoint->endpoint_number;
            msd->in_endpoint_max_packet_size = endpoint->max_packet_size;
          }
          endpoint++;
        }
        msd->interface_number = interface->interface_number;
        break;
      } else {
        for(size_t j = 0; j < interface->endpoint_count; j++){
          endpoint++;
        }
      }

      interface = (USB_Interface_Descriptor *)endpoint;
    }

    if(msd->in_endpoint != 0 && msd->out_endpoint != 0){
      configuration_value = config->configuration_value;
      break;
    } 
  }

  if(msd->out_endpoint == 0){
    klog_error("failed to find valid mass storage device protocol for usb device");
    return 0;
  }

  device->device_address = 1;
  msd->device_number = device->device_address;
  USB_Device_Request set_address_request = {};
  set_address_request.request = USB_REQUEST_SET_ADDRESSS;
  set_address_request.value_low = device->device_address;
  if(ehci_control_transfer_without_data(hc, 0, &set_address_request) == 0){
    klog_error("failed to set usb device address");
    return 0;
  }


  if(configuration_value != 0){
    USB_Device_Request set_config_request = {};
    set_config_request.direction = 0;
    set_config_request.request = USB_REQUEST_SET_CONFIGURATION;
    set_config_request.value_low = configuration_value;
    if(ehci_control_transfer_without_data(hc, device->device_address, &set_config_request) == 0){
      klog_error("failed to set usb device configuration");
      return 0;
    }
  } else {
    klog_error("no valid configuration found for usb device");
    return 0;
  }

  klog_debug("usb_device_info:");
  klog_debug(" device_address: 0x%X", (uint64_t)device->device_address);
  klog_debug(" device_class: 0x%X", (uint64_t)device->device_class);
  klog_debug(" vendor_id: 0x%X", (uint64_t)device->vendor_id);
  klog_debug(" product_id: 0x%X", (uint64_t)device->product_id);
  klog_debug(" vendor_string: %.*s", device->vendor_string_length, device->vendor_string);
  klog_debug(" product_string: %.*s", device->product_string_length, device->product_string);

  klog_debug("out_endpoint: %u", (uint32_t)msd->out_endpoint);
  klog_debug("in_endpoint: %u", (uint32_t)msd->in_endpoint);
  klog_debug("out_max_packet_size: %u", (uint32_t)msd->out_endpoint_max_packet_size);
  klog_debug("in_max_packet_size: %u", (uint32_t)msd->in_endpoint_max_packet_size);

  USB_Device_Request msd_reset_request = {};
  msd_reset_request.recipient = USB_REQUEST_RECIPIENT_INTERFACE;
  msd_reset_request.mode = USB_REQUEST_MODE_CLASS;
  msd_reset_request.request = 0xFF;
  msd_reset_request.index_high = msd->interface_number >> 8;
  msd_reset_request.index_low = msd->interface_number;
  if(ehci_control_transfer_without_data(hc, device->device_address, &msd_reset_request) == 0){
    klog_error("usb mass storage device failed bulk reset");
  }

  uint8_t inquiry_buffer[36] = {};
  SCSI_Inquiry_Command inquiry_command = {};
  scsi_create_inquiry(&inquiry_command, 36);
  if(ehci_bulk_transfer_with_data(hc, msd, &inquiry_command.cbw, sizeof(inquiry_command), inquiry_buffer, sizeof(inquiry_buffer)) == 0){
    klog_error("inquiry bulk transfer error!");
    return 0;
  }

  SCSI_Inquiry_Data *inquiry_data = (SCSI_Inquiry_Data *)inquiry_buffer;
  klog_debug("version: %u", (uint32_t)inquiry_data->version);

  SCSI_Test_Unit_Ready_Command test_unit_ready_command = {
    .cbw.signature = USB_CBW_SIGNATURE,
    .cbw.tag = 0xFFFF0000,
    .cbw.length = 6
  };

  if(ehci_bulk_transfer_no_data(hc, msd, &test_unit_ready_command.cbw) == 0){
    klog_error("test_unit_ready failed for usb_device");
    return 0;
  }

  SCSI_Read_Capacity_Data read_capacity_data = {};
  SCSI_Read_Capacity_Command read_capacity_command = {
    .cbw.signature = USB_CBW_SIGNATURE,
    .cbw.tag = 0x25,
    .cbw.transfer_length = 0x08,
    .cbw.direction = 1, //device_to_host
    .cbw.length = 10,
    .operation_code = 0x25,
  };

  if(ehci_bulk_transfer_with_data(hc, msd, &read_capacity_command.cbw, 31, &read_capacity_data, sizeof(read_capacity_data)) == 0){
    klog_error("failed to execute read capacity command!");
    return 0;
  }

  msd->logical_block_count = read_capacity_data.logical_block_address_0;
  msd->logical_block_count |= read_capacity_data.logical_block_address_1 << 8;
  msd->logical_block_count |= read_capacity_data.logical_block_address_2 << 16;
  msd->logical_block_count |= read_capacity_data.logical_block_address_3 << 24;
  msd->logical_block_count += 1; //read_capacity holds the last addressable block
  msd->logical_block_size = read_capacity_data.block_size_0;
  msd->logical_block_size |= read_capacity_data.block_size_1 << 8; 
  msd->logical_block_size |= read_capacity_data.block_size_2 << 16; 
  msd->logical_block_size |= read_capacity_data.block_size_3 << 24;
  klog_debug("%lu Blocks, %lu Bytes", msd->logical_block_count, msd->logical_block_count * msd->logical_block_size);

  uint8_t read_buffer[1024] = {};
  const uintptr_t buffer_physical_address = (uintptr_t)read_buffer;
  if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, 0x00, 1) == 0){
    klog_error("failed to read msd partition table");
    return 0;
  }

  MBR_Partition_Table *pt = (MBR_Partition_Table *)(read_buffer + 0x1BE);
  for(size_t partition_index = 0; partition_index < 4; partition_index++){
    uint64_t *raw_data = (uint64_t *)pt;
    if(raw_data[0] == 0 && raw_data[1] == 0) continue;
    if(pt->system_id != PARTITION_TYPE_LINUX_FILESYSTEM)
      klog_error("currently only support for linux fs");

    Partition_Info partition_info = {};
    partition_info.partition_type = pt->system_id;
    partition_info.first_block = pt->start_sector;
    partition_info.block_count = pt->sector_count;
    klog_debug("partiton_start_block: %u", (uint32_t)partition_info.first_block);
    klog_debug("partition_block_count: %u", (uint32_t)partition_info.block_count);

    //Read the EXT2 Superblock 

    Ext2_Filesystem *extfs = &g_ext2;
    uint32_t superblock_location = partition_info.first_block + 2;
    klog_debug("superblock_location: %u", superblock_location);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, superblock_location, 1) == 0){
      klog_error("failed to read superblock from ext2 partition");
      return 0;
    }

    Ext2_Superblock *superblock = (Ext2_Superblock *)read_buffer;
    if(superblock->ext2_signature != EXT2_SUPERBLOCK_SIGNATURE){
      klog_error("superblock has invalid signature");
      return 0;
    } 
    if(superblock->version_major < 1){
      klog_error("ext2 uses version 0");
    }

    extfs->partition_first_sector = partition_info.first_block;
    extfs->inode_size = superblock->inode_struct_size;
    extfs->block_size = 1024 << superblock->block_size_shift_count;
    extfs->inode_count_per_group = superblock->inodes_per_group;
    extfs->buffer_physical_address = extfs->buffer_physical_address;
    extfs->superblock_sector = superblock_location;
    strict_assert(buffer_physical_address  + 4096 < 0x200000);
    uint32_t block_group_count = superblock->block_count / superblock->blocks_per_group;

    if(extfs->block_size % msd->logical_block_size != 0) { klog_error("filesystem block size not multiple of device logical block size"); }
    extfs->sectors_per_block = extfs->block_size / msd->logical_block_size;
    ext2_log_fs_info(extfs);

    klog_debug("inode_count: %u", (uint32_t)superblock->inode_count);
    klog_debug("block_count: %u", (uint32_t)superblock->block_count);
    klog_debug("block_group_count: %u", block_group_count);
    klog_debug("unallocated_inodes: %u", superblock->unallocated_inodes);
    klog_debug("unallocated_blocks: %u", superblock->unallocated_blocks);

    uint32_t current_block_count = superblock->block_count - superblock->unallocated_blocks;
    uint32_t current_inode_count = superblock->inode_count - superblock->unallocated_inodes;
    klog_debug("allocated_blocks: %u", current_block_count);
    klog_debug("allocated_inodes: %u", current_inode_count);
    klog_debug("volume_name: %s", superblock->volume_name);

    static const uint16_t EXT2_INODE_TYPE_DIRECTORY = 0x4000;
    uint32_t block_group_descriptor_table_sector = ext2fs_get_sector_location(1, extfs); 
    klog_debug("bock_group_descriptor_table_sector: %u", block_group_descriptor_table_sector);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, block_group_descriptor_table_sector, 1) == 0){
      klog_error("failed to read block_group_descriptor_table");
      return 0;
    }

    Ext2_Block_Group_Descriptor *block_group_descriptor = (Ext2_Block_Group_Descriptor *)read_buffer;
    klog_debug("directory_count: %u", (uint32_t)block_group_descriptor->directory_count);
    klog_debug("unallocated_block_count: %u", (uint32_t)block_group_descriptor->unallocated_block_count);
    klog_debug("unallocated_inode_count: %u", (uint32_t)block_group_descriptor->unallocated_inode_count);
    uint32_t allocated_block_count = superblock->blocks_per_group - block_group_descriptor->unallocated_block_count;
    uint32_t allocated_inode_count = superblock->inodes_per_group - block_group_descriptor->unallocated_inode_count;
    klog_debug("allocated_block_count: %u", allocated_block_count);
    klog_debug("allocated_inode_count: %u", allocated_inode_count);

    ext2_read_inode(2, extfs);

    uint32_t inode_table_sector = ext2fs_get_sector_location(block_group_descriptor->inode_table_block_number, extfs);
    klog_debug("inode_table_sector: %u", inode_table_sector);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, inode_table_sector, 1) == 0){
      klog_error("failed to read inode table");
      return 0;
    }

    Ext2_Inode *inode_table = (Ext2_Inode *)read_buffer;
    Ext2_Inode *root_inode = (Ext2_Inode *)(read_buffer + extfs->inode_size);
    uint32_t directory_entries_location = ext2fs_get_sector_location(root_inode->direct_block_pointer_0, extfs);
    klog_debug("directory_entries_location: %u", directory_entries_location);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, directory_entries_location, 1) == 0){
      klog_debug("failed to read directory entriies");
      return 0;
    }

    Ext2_Directory_Entry *directory_entry = (Ext2_Directory_Entry *)read_buffer; 
    while(((uintptr_t)directory_entry - (uintptr_t)read_buffer) < 4096){
      klog_debug("directory_name: %.*s", directory_entry->name_length, directory_entry->name);
      if(directory_entry->entry_size == 0) break;
      directory_entry = (Ext2_Directory_Entry *)((uintptr_t)directory_entry + directory_entry->entry_size);
    }


    pt++;
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
  lapic_wait_milliseconds(50);
  klog_debug("waited 50ms");
  *port = *port & ~EHCI_PORT_RESET_BIT;
  //NOTE(Torin 2016-09-25) The HC has 2ms to set RESET bit to 0
  wait_for_condition((*port & EHCI_PORT_RESET_BIT) == 0, 2){}
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

  EHCI_Controller *hc = &g_ehci;
  hc->pci_device = *pci_device;
  EHCI_Capability_Registers *cap_regs = (EHCI_Capability_Registers *)ehci_virtual_address;
  uintptr_t operational_register_base = ehci_virtual_address + cap_regs->capability_length;
  EHCI_Operational_Registers *op_regs = (EHCI_Operational_Registers *)operational_register_base;
  hc->op_regs = op_regs;

  bool use_64_bit_ds = false;
  size_t port_count = 0;
  bool is_port_power_control_enabled = false;
  
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
        wait_for_condition(pci_read_uint32() & USBLEGSUP_OS_OWNERSHIP, 2){
          klog_error("failed to take os owernership of echi controller");
          return 0;
        }
        wait_for_condition((pci_read_uint32() & USBLEGSUP_BIOS_OWNERSHIP) == 0, 2){
          klog_error("bios faild to release owenership of ehci controller");
          return 0;
        }
      }
    }
  }

  { //NOTE(Torin 2016-09-17) Extract HCSPARAMS information
    static const uint32_t HCS_PORT_COUNT_MASK = 0b1111;
    static const uint32_t POWER_PORT_CONTROL_BIT = 1 << 4;
    port_count = cap_regs->hcs_params & HCS_PORT_COUNT_MASK; 
    is_port_power_control_enabled = cap_regs->hcs_params & POWER_PORT_CONTROL_BIT;
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
    for(size_t i = 0; i < 1024; i++) periodic_frame_list[i] = (uint32_t)(uintptr_t)periodic_qh | EHCI_POINTER_TYPE_QH | EHCI_POINTER_TERMINATE;
  }

  { //NOTE(Torin 2016-09-13) Set nessecary registers and initalize the controller 
    static const uint32_t USBCMD_RUN_STOP = (1 << 0);
    static const uint32_t USBCMD_HCRESET = 1 << 1;
    static const uint32_t USBCMD_PERIODIC_SCHEDULE_ENABLE = (1 << 4);
    static const uint32_t USBCMD_ASYNCH_SCHEDULE_ENABLE = (1 << 5);
    static const uint32_t USBCMD_INTERRUPT_THRESHOLD_CONTROL_8 = (0x08 << 16); //1ms
    static const uint32_t USBSTATUS_CONTROLLER_HALTED = (1 << 12);
    static const uint32_t USBSTATUS_HOST_SYSTEM_ERROR = 1 << 4;

    static const uint32_t USBINTR_INTERRUPT_ENABLE = 1 << 0;
    static const uint32_t USBINTR_ERROR_INTERRUPT_ENABLE = 1 << 1;
    static const uint32_t USBINTR_PORT_CHANGE_INTERRUPT_ENABLE = 1 << 2;
    static const uint32_t USBINTR_FRAME_LIST_ROLLOVER_ENABLE = 1 << 3;
    static const uint32_t USBINTR_HOST_SYSTEM_ERROR_ENABLE = 1 << 4;
    static const uint32_t USBINTR_INTERRUPT_ON_ASYNC_ADVANCE_ENABLE = 1 << 5;

    //NOTE(Torin 2016-10-09)The HC will halt within 16 microframes of clearing the bit
    op_regs->usb_command = op_regs->usb_command & (~USBCMD_RUN_STOP);
    //NOTE(Torin 2016-10-09) 16 micro-frames * 125micro-seconds = 2mili-seconds
    wait_for_condition((op_regs->usb_command & USBCMD_RUN_STOP) == 0, 3) { return 0;}
    op_regs->usb_command = op_regs->usb_command | USBCMD_HCRESET;
    //TODO(Torin 2016-10-09) What is a good value to wait while the HC resets?
    wait_for_condition((op_regs->usb_command & USBCMD_HCRESET) == 0, 60) { return 0; }

    op_regs->usb_interrupt = USBINTR_INTERRUPT_ENABLE | USBINTR_ERROR_INTERRUPT_ENABLE | USBINTR_PORT_CHANGE_INTERRUPT_ENABLE |
      USBINTR_FRAME_LIST_ROLLOVER_ENABLE | USBINTR_HOST_SYSTEM_ERROR_ENABLE | USBINTR_INTERRUPT_ON_ASYNC_ADVANCE_ENABLE;
    op_regs->ctrl_ds_segment = 0x00;
    op_regs->perodic_list_base = (uint32_t)(uintptr_t)g_ehci.periodic_frame_list;
    op_regs->async_list_address = (uint32_t)(uintptr_t)&g_ehci.asynch_qh;
    op_regs->frame_index = 0x00;
    op_regs->config_flag = 1; //NOTE(Torin) Route all ports to the EHCI controler(Rather than compainion controllers)
    op_regs->usb_command = USBCMD_INTERRUPT_THRESHOLD_CONTROL_8 | USBCMD_ASYNCH_SCHEDULE_ENABLE | USBCMD_RUN_STOP; 
    wait_for_condition((op_regs->usb_status & USBSTATUS_CONTROLLER_HALTED) == 0, 2) { return 0; }
    klog_debug("ehci controller was started");
  }

  lapic_wait_milliseconds(2); //TODO(Torin 2016-10-06) Check what the right value for this wait should be
  //Is this where we should wait 100ms for the ports power state to stabialize?

  { //NOTE(Torin 2016-09-06) Enumerating the ports managed by this controler
    for(size_t port_index = 0; port_index < port_count; port_index++) {
      static const uint32_t PORT_ENABLED_BIT = (1 << 2);
      static const uint32_t PORT_POWER_BIT = 1 << 12;
      static const uint32_t PORT_ENABLED_STATUS_CHANGED_BIT = 1 << 3;
      static const uint32_t PORT_CONNECT_STATUS_CHANGED_BIT = 1 << 1;
      static const uint32_t PORT_ENALBED_STATUS_BIT = 1 << 2;
      static const uint32_t PORT_CONNECT_STATUS_BIT = 1 << 0;
      static const uint32_t PORT_LINE_STATUS_LOW_SPEED_DEVICE = 1 << 10;

     if(ehci_is_hc_halted(op_regs)){
       klog_error("ehci host controller unexpectedly halted during port enumeration");
       return 0;
     }

      static const uint32_t USBSTS_PORT_CHANGE_DETECT = 1 << 2;
      op_regs->usb_status = op_regs->usb_status & ~USBSTS_PORT_CHANGE_DETECT;

      uint32_t volatile *port_reg = &op_regs->ports[port_index];
      if(*port_reg & PORT_CONNECT_STATUS_CHANGED_BIT){
        *port_reg = *port_reg | PORT_CONNECT_STATUS_CHANGED_BIT; //clears out connect status change
        wait_for_condition((*port_reg & PORT_CONNECT_STATUS_CHANGED_BIT) == 0, 1) {}
        if(*port_reg & PORT_CONNECT_STATUS_BIT){
          lapic_wait_milliseconds(100);
          klog_debug("USB_Device connected on port %lu", port_index);
          bool is_port_enabled = *port_reg & PORT_ENABLED_BIT;
          if(is_port_enabled == false) {
            if(*port_reg & PORT_LINE_STATUS_LOW_SPEED_DEVICE){
              klog_error("port is a low speed device!");
            }
            kassert(ehci_is_hc_halted(op_regs) == 0);
            is_port_enabled = ehci_reset_port(port_reg);
          }
          if(is_port_enabled == false){
            klog_debug("Port %lu failed to enable", port_index);
          } else {
            klog_debug("Port %lu enabled sucuessfuly", port_index);
            lapic_wait_milliseconds(10);
            //TODO(Torin 2016-10-15) Put this inside of the EHCI controler struct!
            USB_Device device = {};
            ehci_initalize_device(&g_ehci, &device);
          }
        }
      }
      klog_debug("no device connected on port %lu", port_index);
    }
  }
  klog_debug("echi was initalized");
  return 1;
}
