//NOTE(Torin) The flowing descriptors assume a long-mode only enviroment

//NOTE(Torin) Global Descriptor Table
#if 0
typedef struct {
  union {
    struct {
      uint32_t ignored0;
      uint8_t ignored1;
      uint8_t ignored2 : 1;
      uint8_t ignored3 : 1;
      uint8_t 



    };
    uint64_t packed;
  };
} GDT_Entry;
#endif

//NOTE(Torin) Interrupt Descriptor Table
typedef struct {
	uint16_t offset_0_15;
	uint16_t code_segment_selector;
	uint8_t ist;
	uint8_t type_and_attributes;
	uint16_t offset_16_31;
	uint32_t offset_32_63;
	uint32_t null_uint32;
} IDT_Entry;

//NOTE(Torin) Task-State Segment
struct TSS64_Struct {
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint32_t reserved3;
  uint32_t io_map_base_address;
} __attribute__((packed));

typedef struct TSS64_Struct TSS_64;
static TSS_64 g_tss_entry;

#define GDT_DESCRIPTOR_TYPE_LDT (0b0010)
#define GDT_DESCRIPTOR_TYPE_TSS (0b1001)

static inline
void gdt_encode_system_descriptor(uintptr_t base_address, uint32_t segment_limit, uint8_t type, uint8_t privlege_level, uintptr_t out){
  kassert(segment_limit < (1 << 19));
  kassert(type < (1 << 4));
  kassert(privlege_level < (1 << 2));

  struct GDT_System_Descriptor {
    uint16_t segment_limit_0_15;
    uint16_t base_address_0_15;
    uint8_t base_address_16_23;
    uint8_t type : 4;
    uint8_t zero : 1;
    uint8_t privlege_level : 2;
    uint8_t present : 1;
    uint8_t segment_limit_16_19 : 4;
    uint8_t available : 1;
    uint8_t reserved0 : 2; 
    uint8_t granularity : 1;
    uint16_t base_address_24_31;
    uint32_t base_address_32_63;
    uint32_t reserved1;
  } __attribute__((packed));

  struct GDT_System_Descriptor descriptor = {};
  descriptor.base_address_0_15  = (base_address >> 0) & 0xFFFF;
  descriptor.base_address_16_23 = (base_address >> 16) & 0xFF;
  descriptor.base_address_24_31 = (base_address >> 24) & 0xFF;
  descriptor.base_address_32_63 = (base_address >> 32) & 0xFFFFFFFF;

  descriptor.segment_limit_0_15 = (segment_limit >> 0) & 0xFFFF;
  descriptor.segment_limit_16_19 = (segment_limit >> 16) & 0b1111;

  descriptor.present = 1;
  descriptor.available = 1;
  descriptor.type = type;
  descriptor.privlege_level = privlege_level;
  memcpy(out, &descriptor, sizeof(struct GDT_System_Descriptor));

  klog_debug("tss base address: 0x%X", base_address);
  klog_debug("tss limit: 0x%X", segment_limit);
}

static void
idt_encode_entry(const uintptr_t entry_addr, const uintptr_t handler_addr, const bool is_user_accessible){
	static const uint64_t PRIVILEGE_LEVEL_0 = 0b00000000;
	static const uint64_t PRIVILEGE_LEVEL_3 = 0b01100000;
	static const uint64_t PRESENT_BIT = (1 << 7); 
	
	static const uint64_t TYPE_TASK_GATE_64 = 0x5;
	static const uint64_t TYPE_INTERRUPT_GATE_64 = 0xE;
	static const uint64_t TYPE_TRAP_GATE_64 = 0xF;

  IDT_Entry *entry = (IDT_Entry *)entry_addr;
	entry->offset_0_15 = (uint16_t)(handler_addr & 0xFFFF);
	entry->offset_16_31 = (uint16_t)((handler_addr >> 16) & 0xFFFF);
	entry->offset_32_63 = (uint32_t)((handler_addr >> 32) & 0xFFFFFFFF);
	entry->type_and_attributes = PRESENT_BIT | TYPE_INTERRUPT_GATE_64 | (is_user_accessible ?  PRIVILEGE_LEVEL_3 : PRIVILEGE_LEVEL_0); 
	entry->code_segment_selector = GDT_RING0_CODE;
	entry->ist = 0;
}

static inline
void gdt_gdtr(uintptr_t base_address, size_t size){

}

static inline
void tss_ldr(const uint16_t selector_index){
  asm volatile("cli");

  struct TSS_Selector {
    union {
      struct {
        uint16_t requestor_prilvilege_level : 2;
        uint16_t table_indicator : 1; //NOTE(Torin) Must always be zero
        uint16_t selector_index : 13;
      };
       uint16_t packed;
    };
  } task_register;
  
  //@Research
  //TODO(Torin: 2016-08-24) AMD64 system manual has the task_register with the above structure
  //It appears that is incorrect and its just a flat selector_index.  Make sure this is the case!
  uint16_t value = selector_index | 0b00; //This is probably what was ment?
  klog_debug("task_register selector_index: 0x%X", (uint32_t)value);
  //asm volatile ("ltr %0" : : "a"(task_register.packed));
  asm volatile ("ltr %0" : : "a"(value));
  asm volatile("sti");
}

//NOTE(Torin) This is the only possible value for a longmode gdt data segment entry
#define GDT_DATA_SEGMENT_64 ((1L << 47) | (1L << 44))
#define GDT_CODE_SEGMENT_64(ring) ((1L << 53) | (1L << 47) | ((uint64_t)(ring & 0b11) << 45) | (1L << 44) | (1L << 43))

#if 0
uint64_t g_GDT[] = {
  0x0000000000000000,     //null entry:  0x00
  GDT_DATA_SEGMENT_64,    //longmode data segment:   0x08  
  GDT_CODE_SEGMENT_64(0), //longmode kernel code segment: 0x10
  GDT_CODE_SEGMENT_64(3), //longmode usermode code segment: 0x18
  //longmode tss descriptor: 0x20
};
#endif

#if 0
GDTR_Register g_gdtr = {
  .limit = sizeof(g_GDT),
  .base = &g_GDT[0]
};
#endif
