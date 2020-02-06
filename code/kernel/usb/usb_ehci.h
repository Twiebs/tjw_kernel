
typedef struct {
  uint32_t is_64_bit                        : 1; // 0
  uint32_t must_use_1024_size_frame_list    : 1; // 1
  uint16_t supports_park_feature            : 1; // 2
  uint32_t reserved0                        : 1; // 3
  uint32_t isochronous_scheduling_threshold : 4; // 4-7
  uint64_t extended_capabilities_pointer    : 8; // 8-15
  uint32_t reserved1                        : 16; // 16-31
} __attribute((packed)) EHCI_Host_Controller_Capability_Parameters;
static_assert(sizeof(EHCI_Host_Controller_Capability_Parameters) == 4);

typedef struct {
  uint8_t capability_length;
  uint8_t reserved;
  uint16_t hci_version;
  uint32_t hcs_params;
  EHCI_Host_Controller_Capability_Parameters hcc_params;
  uint64_t hcsp_port_route;
} __attribute((packed)) EHCI_Capability_Registers;
static_assert(sizeof(EHCI_Capability_Registers) == 20);

typedef struct {
  uint32_t usb_command;
  uint32_t usb_status;
  uint32_t usb_interrupt;
  uint32_t frame_index;
  uint32_t ctrl_ds_segment;
  uint32_t perodic_list_base;
  uint32_t async_list_address;
  uint32_t reserved[9];
  uint32_t config_flag;
  uint32_t ports[0];
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

typedef struct {
  volatile uint32_t next_td;
  volatile uint32_t alt_next_td;
  volatile uint32_t qtd_token;
  volatile uint32_t buffer_pointer_low[5];
  volatile uint32_t buffer_pointer_high[5];
  volatile uint8_t padding[12]; //QTD Must have 64 byte alignment
} __attribute((packed)) EHCI_QTD; //64 bytes

//TODO(Torin 2016-10-21) Does the QH need to be 32byte algined or literaly cache line aligned?
//Or can it be aligned on 32Byte boundry (96Byte struct size)???
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
  volatile uint8_t padding[60];
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


typedef struct {
  uint32_t periodic_frame_list[1024]; //4096 bytes
  //====================================================== PAGE BOUNDRAY
  EHCI_Queue_Head asynch_qh   __attribute((aligned(64)));   //128 Bytes 
  EHCI_Queue_Head periodic_qh __attribute((aligned(64)));   //128 Bytes
  EHCI_QTD qtd_array[4]       __attribute((aligned(64)));   //256 Bytes (4*64)
  uintptr_t first_page_physical_address;
  uintptr_t second_page_physical_address;

  volatile EHCI_Operational_Registers *op_regs;
  volatile EHCI_Capability_Registers *cap_regs;

  PCI_Device pci_device;

  uint8_t port_count;
  bool is_64bit_capable;
  bool has_port_power_control;
} EHCI_Controller;

static_assert(sizeof(EHCI_Controller) < 4096*2);

int ehci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);
Error_Code ehci_read_to_physical_address(EHCI_Controller *hc, USB_Mass_Storage_Device *msd, uintptr_t out_data, uint32_t start_block, uint16_t block_count);