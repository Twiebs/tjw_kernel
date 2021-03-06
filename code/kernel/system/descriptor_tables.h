
//NOTE(Torin) Task-State Segment
typedef struct {
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
} __attribute((packed)) Task_State_Segment;

static const uint8_t GDT_DESCRIPTOR_TYPE_LDT = 0b0010;
static const uint8_t GDT_DESCRIPTOR_TYPE_TSS = 0b1001;

//NOTE(Torin: 2017-07-26) Offsets to each descriptor encoded in
//the GDT table that is setup in assembly.  0x00 is a null descriptor
static const uintptr_t GDT_RING0_DATA_ENTRY_OFFSET = 0x08;
static const uintptr_t GDT_RING3_DATA_ENTRY_OFFSET = 0x10;
static const uintptr_t GDT_RING0_CODE_ENTRY_OFFSET = 0x18;
static const uintptr_t GDT_RING3_CODE_ENTRY_OFFSET = 0x20;
static const uintptr_t GDT_TSS_ENTRY_OFFSET = 0x28;


void gdt_encode_system_descriptor(uintptr_t base_address, uint32_t segment_limit, uint8_t type, uint8_t privlege_level, uintptr_t out);
void tss_ldr(const uint16_t selector_index);