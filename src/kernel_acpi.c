//Handles parsing headers for ACPI(Advanced Configuration and Power Interface)

//Legacy Root System Descriptor Pointer 
typedef struct {
  uint8_t signature[8];
	uint8_t check_sum;
	uint8_t oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
} __attribute__((packed)) RSDP_Descriptor_1;

//Root System Descriptor Pointer
typedef struct {
	RSDP_Descriptor_1 first_part;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__((packed)) RSDP_Descriptor_2;

//System Descriptor Table
typedef struct {
	uint32_t signature;
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[6];
	uint64_t oem_table_id;
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed)) ACPI_SDT_Header;

typedef struct {
	uint32_t signature;
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[6];
	uint64_t oem_table_id;
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
	uint32_t local_controler_address;
	uint32_t flags;
} __attribute__((packed)) MADT_Header;

typedef struct {
	uint8_t entry_type;
	uint8_t entry_length;
} MADT_Entry;

typedef struct  {
	uint8_t acpi_processor_id;
	uint8_t apic_id;
	uint32_t flags;
} MADT_Entry_Processor_Local_APIC;

typedef struct {
	uint8_t io_apic_id;
	uint8_t reserved;
	uint32_t io_apic_address;
	uint32_t global_system_interrupt_base;
} MADT_Entry_IO_APIC;

typedef struct {
	uint8_t bus_source;
	uint8_t irq_source;
	uint32_t global_system_interrupt;
	uint16_t flags;
} MADT_Entry_Interrupt_Source_Override;

static const uint8_t MADT_ENTRY_TYPE_PROCESSOR_LOCAL_APIC = 0;
static const uint8_t MADT_ENTRY_TYPE_IO_APIC = 1;
static const uint8_t MADT_ENTRY_TYPE_INTERRUPT_SOURCE_OVERRIDE = 2;

void parse_root_system_descriptor(RSDP_Descriptor_1 *rsdp) {
	if (rsdp->signature[0] == 'R' &&
			rsdp->signature[1] == 'S' &&
			rsdp->signature[2] == 'D' &&
			rsdp->signature[3] == ' ' &&
			rsdp->signature[4] == 'P' &&
			rsdp->signature[5] == 'T' &&
			rsdp->signature[6] == 'R' &&
			rsdp->signature[7] == ' ') 
	{
		static const uint32_t ACPI_MADT_SIGNATURE = ('C' << 24) | ('I' << 16) | ('P' << 8) | ('A');
		klog_debug("rsdp is valid, rsdt physical address: %u", rsdp->rsdt_address);

		uint64_t virtual_offset = 0;
		uint64_t virtual_page = silly_page_map(rsdp->rsdt_address, &virtual_offset, false);
		uint64_t virtual_address = (virtual_page + virtual_offset);
		ACPI_SDT_Header *rsdt = (ACPI_SDT_Header *)(virtual_address);
    if(rsdt->signature == 0) { klog_debug("fooooobarrrr"); }


#if 0
		if (acpi_header->signature == ACPI_MADT_SIGNATURE) {
			klog_debug("found MADT table");
		}
#endif

	} 
	
	else 
	{
		klog_debug("rsdp->is invalid!");
	}

  klog_debug("parsed root_system_descriptor");
}


