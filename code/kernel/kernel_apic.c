static inline void ioapic_write_register(const uintptr_t ioapic_base, const uint8_t offset, const uint32_t value);
static inline uint32_t ioapic_read_register(const uintptr_t ioapic_base, const uint8_t offset);
static inline void lapic_write_register(uintptr_t lapic_base, uintptr_t register_offset, uint32_t value);

struct IOAPIC_IRQR_LOW_STRUCT {
  union {
    struct {
      uint8_t vector; // 0-7
      uint8_t delivery_mode : 2; //8-10
      uint8_t destination_mode : 1; //11
      uint8_t delivery_status : 1; // 12
      uint8_t pin_polarity : 1; // 13
      uint8_t remote_irr : 1; //14
      uint8_t trigger_mode : 1; //15
      uint8_t mask : 1; //16
      uint16_t reserved : 15; 
    };
    uint32_t packed;
  };
} __attribute__((packed));

struct IOAPIC_IRQR_HIGH_STRUCT {
  union {
    struct {
      uint32_t reserved    : 24;
      uint32_t destination :  8;
    };
    uint32_t packed;
  };
} __attribute__((packed));

typedef struct IOAPIC_IRQR_LOW_STRUCT IOAPIC_IRQR_LOW;
typedef struct IOAPIC_IRQR_HIGH_STRUCT IOAPIC_IRQR_HIGH;


static inline
void ioapic_write_register(const uintptr_t ioapic_base, const uint8_t offset, const uint32_t value){
  uint32_t volatile *ioapic = (uint32_t volatile *)ioapic_base;
  ioapic[0] = offset;
  ioapic[4] = value;
}

static inline
uint32_t ioapic_read_register(const uintptr_t ioapic_base, const uint8_t offset){
  uint32_t volatile *ioapic = (uint32_t volatile*)ioapic_base;
  ioapic[0] = offset;
  return ioapic[4];
}

static inline
void lapic_write_register(uintptr_t lapic_base, uintptr_t register_offset, uint32_t value){
  uint32_t volatile *lapic = (uint32_t volatile *)(lapic_base + register_offset);
  lapic[0] = value;
}

static inline
uint32_t lapic_read_register(uintptr_t lapic_base, uintptr_t register_offset){
  uint32_t volatile *lapic_register = (uint32_t volatile *)(lapic_base + register_offset);
  return lapic_register[0];
}

static void
ioapic_initalize(uintptr_t ioapic_register_base) {
  asm volatile("cli");
  static const uint8_t DELIVERY_MODE_FIXED = 0x00;
  //NOTE(Torin) Keyboard
  IOAPIC_IRQR_LOW low = {};
  IOAPIC_IRQR_HIGH high = {};
  low.vector = 0x21;
  ioapic_write_register(ioapic_register_base, 0x12, low.packed);
  ioapic_write_register(ioapic_register_base, 0x13, high.packed);
  asm volatile("sti");
}

static inline
uint32_t lapic_get_id(uintptr_t lapic_virtual_address){
  static const uint32_t APIC_ID_REGISTER = 0x20;
  uint32_t result = lapic_read_register(lapic_virtual_address, APIC_ID_REGISTER);
  return result;
}

static void
lapic_configure_one_shot_timer(uintptr_t lapic_virtual_address, uint32_t inital_count, uint32_t irq_number){
  static const uint32_t LAPIC_TIMER_IRQ_REGISTER = 0x320; 
  static const uint32_t LAPIC_TIMER_INITAL_COUNT_REGISTER = 0x380;
  static const uint32_t LAPIC_TIMER_DIVIDE_CONFIG_REGISTER = 0x3E0;
  lapic_write_register(lapic_virtual_address, LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, 0x00); 
  lapic_write_register(lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, inital_count);
  lapic_write_register(lapic_virtual_address, LAPIC_TIMER_IRQ_REGISTER, irq_number);
}

#if 0
static void
lapic_configure_timer(uintptr_t lapic_virtual_address, uint32_t inital_count, uint32_t irq_number, uint32_t mode){
  static const uint32_t TIMER_IRQ_REGISTER = 0x320;
  
  static const uint32_t TIMER_INITAL_COUNT_REGISTER  = 0x380;
  static const uint32_t TIMER_CURRENT_COUNT_REGISTER = 0x390;
  static const uint32_t TIMER_DIVIDE_CONFIG_REGISTER = 0x3E0;

  static const uint32_t TIMER_DIVIDE_BY_2 = 0b00;
  static const uint32_t TIMER_DIVIDE_BY_4 = 0b01;
  static const uint32_t TIMER_DIVIDE_BY_8 = 0b10;
  static const uint32_t TIMER_DIVIDE_BY_16 = 0b11;

  static const uint32_t TIMER_ONE_SHOT_MODE = 0x00000;
  static const uint32_t TIMER_PERIODIC_MODE = 0x20000;
  const uint32_t mode_mask = mode ? TIMER_PERIODIC_MODE : TIMER_ONE_SHOT_MODE;

  lapic_write_register(lapic_virtual_address, TIMER_INITAL_COUNT_REGISTER, inital_count);
  lapic_write_register(lapic_virtual_address, TIMER_DIVIDE_CONFIG_REGISTER, TIMER_DIVIDE_BY_16);
  lapic_write_register(lapic_virtual_address, TIMER_IRQ_REGISTER, irq_number | mode_mask);
}
#endif

static void
lapic_wait_milliseconds(uint32_t ms){
  globals.lapic_timer_ticks = 0;
  while(globals.lapic_timer_ticks < ms){
    asm volatile("nop"); 
  }
}

//TODO(Torin 2016-10-30) This should be an inlined procedure that only exists in the entry procedure and is 
//run as soon as possible

//NOTE(Torin) Called from the bootstrap processor to send A SIPI signal to the target application processor 
static void lapic_startup_ap(uintptr_t apic_register_base, uint8_t target_apic_id, uint8_t target_page_number){
  static const uintptr_t APIC_ICR1_OFFSET = 0x300;
  static const uintptr_t APIC_ICR2_OFFSET = 0x310;

  static const uint8_t DELIVERY_MODE_FIXED = 0b000;
  static const uint8_t DELIVERY_MODE_LOWEST_PRIORITY = 0b001;
  static const uint8_t DELIVERY_MODE_SMI = 0b010;
  static const uint8_t DELIVERY_MODE_NMI = 0b100;
  static const uint8_t DELIVERY_MODE_INIT = 0b101;
  static const uint8_t DELIVERY_MODE_SIPI = 0b110;

  static const uint8_t DESTINATION_MODE_PHYSICAL = 0;
  static const uint8_t DESTINATION_MODE_LOGICAL = 1;

  static const uint8_t DEILVERY_STATUS_IDLE = 0;
  static const uint8_t DELIVERY_STATUS_PENDING = 1;

  struct ICR1_STRUCT {
    uint8_t vector;                     // 0 - 7
    uint8_t delivery_mode : 3;          // 8 - 10 
    uint8_t destination_mode : 1;       // 11 
    uint8_t delivery_status : 1;        // 12 
    uint8_t reserved0 : 1;              // 13
    uint8_t level : 1;                  // 14
    uint8_t trigger_mode : 1;           //15
    uint8_t reserved1 : 2;              //16 - 17
    uint8_t destination_shorthand : 2;  // 18 -19
    uint16_t reserved2 : 11;            // 20 - 31
  } __attribute((packed));

  struct ICR2_STRUCT {
    uint32_t reserved_0_ : 24;
    uint8_t destination_field;
  } __attribute((packed));

  typedef struct ICR1_STRUCT ICR1_Register;
  typedef struct ICR2_STRUCT ICR2_Register;

  ICR1_Register icr1 = {};
  icr1.delivery_mode = DELIVERY_MODE_INIT;
  ICR2_Register icr2 = {};
  icr2.destination_field = target_apic_id;

  //NOTE(Torin) Writing to the ICR1 causues a IPI to be generated automaticly so ICR2 is written first
  //Send INIT IPI To set processor to wait for SIPI

  uint32_t icr2_value = *(uint32_t *)&icr2;
  uint32_t icr1_value = *(uint32_t *)&icr1;
  klog_debug("icr2_value: %u", icr2_value);
  klog_debug("icr1_value: %u", icr1_value);

  lapic_write_register(apic_register_base, APIC_ICR2_OFFSET, *(uint32_t *)&icr2);
  lapic_write_register(apic_register_base, APIC_ICR1_OFFSET, *(uint32_t *)&icr1);
  while(icr1.delivery_status == 1){
    klog_debug("waiting for icr1 to get delivered!");
  }

  for(size_t i = 0; i < 0xFFFFF; i++) { asm volatile ( "nop" ); }

  //Setup and send SIPI
  icr1.vector = target_page_number; 
  icr1.delivery_mode = DELIVERY_MODE_SIPI;
  lapic_write_register(apic_register_base, APIC_ICR2_OFFSET, *(uint32_t *)&icr2);
  lapic_write_register(apic_register_base, APIC_ICR1_OFFSET, *(uint32_t *)&icr1);
  for(size_t i = 0; i < 0xFFFFF; i++) { asm volatile ( "nop" ); }
  lapic_write_register(apic_register_base, APIC_ICR2_OFFSET, *(uint32_t *)&icr2);
  lapic_write_register(apic_register_base, APIC_ICR1_OFFSET, *(uint32_t *)&icr1);
}