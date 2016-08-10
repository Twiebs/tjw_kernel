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

//MADT: Multiple APIC Description Table
//=====================================
//=======================================

void parse_root_system_descriptor(const RSDP_Descriptor_1 *rsdp, System_Info *sys) {
	if (rsdp->signature[0] == 'R' &&
			rsdp->signature[1] == 'S' &&
			rsdp->signature[2] == 'D' &&
			rsdp->signature[3] == ' ' &&
			rsdp->signature[4] == 'P' &&
			rsdp->signature[5] == 'T' &&
			rsdp->signature[6] == 'R' &&
			rsdp->signature[7] == ' ') 
	{
		klog_debug("rsdp is valid, rsdt physical address: %u", rsdp->rsdt_address);

		uint64_t physical_page, page_offset;
    uint64_t virtual_page = silly_page_map(rsdp->rsdt_address, false, &physical_page, &page_offset);
		uint64_t virtual_address = (virtual_page + page_offset);

		ACPI_SDT_Header *rsdt = (ACPI_SDT_Header *)(virtual_address);
    uint32_t entry_count = (rsdt->length - sizeof(ACPI_SDT_Header)) / 4;
    uint32_t *entries = (uint32_t *)(rsdt + 1);

    for(size_t i = 0; i < entry_count; i++){
      uintptr_t header_physical_addr = (uintptr_t)entries[i];
      int64_t physical_offset = header_physical_addr - physical_page;
      kassert(physical_offset >= 0); //TODO(Torin) This needs to be handled better
      uintptr_t entry_address = virtual_page + physical_offset;
      ACPI_SDT_Header *header = (ACPI_SDT_Header *)entry_address;

      static const uint32_t ACPI_MADT_SIGNATURE = ('C' << 24) | ('I' << 16) | ('P' << 8) | ('A');
      
      if(header->signature == ACPI_MADT_SIGNATURE)
      {
        klog_debug("found MADT table");

        typedef struct {
          ACPI_SDT_Header header;
          uint32_t local_controler_address;
          uint32_t flags;
        } __attribute__((packed)) MADT;
        MADT *madt = (MADT *)header;

        typedef struct {
          uint8_t entry_type;
          uint8_t entry_length;
        } MADT_Entry_Header;

        sys->lapic_register_base = madt->local_controler_address;

        MADT_Entry_Header *madt_entry = (MADT_Entry_Header *)(madt + 1);
        while(((uintptr_t)madt_entry - (uintptr_t)madt) < madt->header.length) 
        {
          typedef struct {
            MADT_Entry_Header header;
            uint8_t acpi_processor_id;
            uint8_t apic_id;
            uint32_t flags;
          } MADT_Entry_Processor_Local_APIC;

          typedef struct {
            MADT_Entry_Header header;
            uint8_t bus_source;
            uint8_t irq_source;
            uint32_t global_system_interrupt;
            uint16_t flags;
          } MADT_Entry_Interrupt_Source_Override;

          enum { 
            PROCESSOR_LOCAL_APIC = 0,
            IO_APIC = 1,
            INTERRUPT_SOURCE_OVERRIDE = 2,
          };
          
          switch(madt_entry->entry_type){
            case PROCESSOR_LOCAL_APIC: {
              MADT_Entry_Processor_Local_APIC * local_apic = (MADT_Entry_Processor_Local_APIC *)madt_entry; 
              kassert(local_apic->flags & 0b1); //Processor must be enabled!
              sys->processor_count++;
            }break;

            case IO_APIC:{
              typedef struct {
                MADT_Entry_Header header;
                uint8_t io_apic_id;
                uint8_t reserved;
                uint32_t io_apic_address;
                uint32_t global_system_interrupt_base;
              } MADT_Entry_IO_APIC;

              
              MADT_Entry_IO_APIC *ioapic = (MADT_Entry_IO_APIC *)madt_entry;
              klog_info("found an IOAPIC");
              klog_info("ioapic id: %u", (uint32_t)ioapic->io_apic_id);
              klog_info("ioapic physical address: %u", ioapic->io_apic_address);

              if(sys->ioapic_register_base != 0){
                klog_info("system contains multiple ioapics");
              } else {
                sys->ioapic_register_base = ioapic->io_apic_address;
              }


            }break;
            case INTERRUPT_SOURCE_OVERRIDE:{
            }break;
          }

          madt_entry = (MADT_Entry_Header *)((uintptr_t)madt_entry + madt_entry->entry_length); 
        }
      } 
    }  
	} 
	
	else 
	{
		klog_debug("rsdp->is invalid!");
	}

  klog_debug("parsed root_system_descriptor");
}


