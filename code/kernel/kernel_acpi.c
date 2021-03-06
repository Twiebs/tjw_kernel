//Handles parsing headers for ACPI(Advanced Configuration and Power Interface)

//Legacy Root System Descriptor Pointer 
typedef struct {
  uint8_t signature[8];
	uint8_t check_sum;
	uint8_t oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
} __attribute((packed)) RSDP_Descriptor_1;

//Root System Descriptor Pointer
typedef struct {
	RSDP_Descriptor_1 first_part;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute((packed)) RSDP_Descriptor_2;

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
} __attribute((packed)) ACPI_SDT_Header;

//ACPI PCI Express memory mapped configuration space 
typedef struct {

} ACPI_MCFG_Table;

//MADT: Multiple APIC Description Table

typedef struct {
  ACPI_SDT_Header header;
  uint32_t local_controler_address;
  uint32_t flags;
} __attribute((packed)) ACPI_MADT;

typedef enum { 
  MADT_Entry_Type_PROCESSOR_LOCAL_APIC = 0,
  MADT_Entry_Type_IO_APIC = 1,
  MADT_Entry_Type_INTERRUPT_SOURCE_OVERRIDE = 2,
} MADT_Entry_Type;

typedef struct {
  uint8_t entry_type;
  uint8_t entry_length;
} __attribute((packed)) MADT_Entry_Header;

typedef struct {
  MADT_Entry_Header header;
  uint8_t processor_id;
  uint8_t apic_id;
  uint32_t flags;
} __attribute((packed)) MADT_Entry_Processor_Local_APIC;

typedef struct {
  MADT_Entry_Header header;
  uint8_t bus_source;
  uint8_t irq_source;
  uint32_t global_system_interrupt;
  uint16_t flags;
} __attribute((packed)) MADT_Entry_Interrupt_Source_Override;

typedef struct {
  MADT_Entry_Header header;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t io_apic_address;
  uint32_t global_system_interrupt_base;
} __attribute((packed)) MADT_Entry_IO_APIC;

static const uint32_t ACPI_MADT_SIGNATURE = ('C' << 24) | ('I' << 16) | ('P' << 8) | ('A');
static const uint32_t ACPI_MCFG_SIGNATURE = ('G' << 24) | ('F' << 16) | ('C' << 8) | ('M');


void acpi_rsdt_parse_madt(ACPI_MADT *madt) {
  System_Info *system = &globals.system_info;
  system->lapic_physical_address = madt->local_controler_address;

  MADT_Entry_Header *madt_entry = (MADT_Entry_Header *)(madt + 1);
  while (((uintptr_t)madt_entry - (uintptr_t)madt) < madt->header.length) {
    switch (madt_entry->entry_type) {
      case MADT_Entry_Type_PROCESSOR_LOCAL_APIC: {
        MADT_Entry_Processor_Local_APIC * local_apic = (MADT_Entry_Processor_Local_APIC *)madt_entry; 
        bool is_apic_enabled = local_apic->flags & 0b1;
        //log_debug(ACPI, ("[acpi] lapic: processor_id: %u apic_id: %u is %s", local_apic->processor_id, local_apic->apic_id, is_apic_enabled ? "enabled" : "disabled");

        if (system->total_cpu_count < ARRAY_COUNT(system->cpu_infos)){
          system->cpu_lapic_ids[system->total_cpu_count] = local_apic->apic_id;
          system->total_cpu_count++;
        } else {
          klog_warning("only 32 cpus are currently supported!");
        }
      } break;

      case MADT_Entry_Type_IO_APIC:{


        
        MADT_Entry_IO_APIC *ioapic = (MADT_Entry_IO_APIC *)madt_entry;
        //klog_info("[acpi] ioapic id: %u", (uint32_t)ioapic->io_apic_id);
        //klog_info("[acpi] ioapic physical address: %u", ioapic->io_apic_address);

        if (system->ioapic_physical_address != 0) {
          klog_warning("[acpi] system contains multiple ioapics");
        } else {
          system->ioapic_physical_address = ioapic->io_apic_address;
        }


      } break;

      case MADT_Entry_Type_INTERRUPT_SOURCE_OVERRIDE: {

      } break;
    }

    madt_entry = (MADT_Entry_Header *)((uintptr_t)madt_entry + madt_entry->entry_length); 
  }
}


void acpi_parse_root_system_descriptor_table(ACPI_SDT_Header *rsdt) {
  System_Info *system = &globals.system_info;
  uint32_t entry_count = (rsdt->length - sizeof(ACPI_SDT_Header)) / 4;
  uint32_t *entries = (uint32_t *)(rsdt + 1);

  for (size_t i = 0; i < entry_count; i++) {
    uintptr_t header_physical_addr = (uintptr_t)entries[i];
    log_debug(ACPI, "qattempting to parse rdst entry at 0x%X", header_physical_addr);
    uintptr_t physical_page_address = header_physical_addr & ~0xFFF;
    uint64_t page_offset = header_physical_addr & 0xFFF;
    memory_map_physical_to_virtual(physical_page_address, globals.memory_state.current_kernel_persistent_virtual_memory_address);
    uintptr_t entry_virtual_address = globals.memory_state.current_kernel_persistent_virtual_memory_address + page_offset;
    ACPI_SDT_Header *header = (ACPI_SDT_Header *)entry_virtual_address;
    uint64_t additional_page_count = (header->length + page_offset) / 4096;
    for (size_t j = 0; j < additional_page_count; j++) {
      memory_map_physical_to_virtual(physical_page_address + (4096 * (j+1)), globals.memory_state.current_kernel_persistent_virtual_memory_address + (4096 * (j+1)));
    }


    //Multiple APIC Description Table
    if (header->signature == ACPI_MADT_SIGNATURE) {
      log_debug(ACPI, "found MADT ACPI table");
      //silly_breakpoint();
      ACPI_MADT *madt = (ACPI_MADT *)header;
      acpi_rsdt_parse_madt(madt);
      //silly_breakpoint();
    } else if (header->signature == ACPI_MCFG_SIGNATURE){
      log_debug(ACPI, "found MCFG ACPI table");
    } else {
      log_debug(ACPI, "skiping rdst entry");
    }


    memory_unmap_virtual_address(globals.memory_state.current_kernel_persistent_virtual_memory_address);
    for (size_t j = 0; j < additional_page_count; j++) {
      memory_unmap_virtual_address(globals.memory_state.current_kernel_persistent_virtual_memory_address + (4096 * (j+1)));
    }
    //silly_breakpoint();
  }
    
  log_debug(ACPI, "Finished parsing RSDT");
}


bool acpi_rsdp_is_signature_valid(RSDP_Descriptor_1 *rsdp) {
  bool result = rsdp->signature[0] == 'R' && rsdp->signature[1] == 'S' &&
      rsdp->signature[2] == 'D' && rsdp->signature[3] == ' ' &&
      rsdp->signature[4] == 'P' && rsdp->signature[5] == 'T' &&
      rsdp->signature[6] == 'R' && rsdp->signature[7] == ' ';
  return result;
}

Error_Code acpi_parse_root_system_descriptor_version1(RSDP_Descriptor_1 *rsdp) {
  if (!acpi_rsdp_is_signature_valid(rsdp)) {
    klog_error("RSDP is invalid!");
    return Error_Code_INVALID_DATA;
  }

  //TODO(Torin) This should be a temporary page!
  uintptr_t physical_address = rsdp->rsdt_address & ~0xFFF;
  uintptr_t page_offset = rsdp->rsdt_address - physical_address;
  //TODO(Torin) Need to figure out how big this should be!
  uintptr_t mapped_virtual_address = memory_map_physical_mmio(physical_address, 4);
  ACPI_SDT_Header *rsdt = (ACPI_SDT_Header *)(mapped_virtual_address + page_offset);
  acpi_parse_root_system_descriptor_table(rsdt);
  return Error_Code_NONE;
}


Error_Code acpi_parse_root_system_descriptor_version2(RSDP_Descriptor_2 *rsdp) {
  log_debug(ACPI, "Parsing RSDT from RSDP2");
  if (!acpi_rsdp_is_signature_valid(&rsdp->first_part)) {
    klog_error("RSDP is invalid!");
    return Error_Code_INVALID_DATA;
  }

  uintptr_t physical_address = rsdp->first_part.rsdt_address & ~0xFFF;
  uintptr_t page_offset = rsdp->first_part.rsdt_address - physical_address;
  uint64_t required_page_count = rsdp->length / 4096;
  if (rsdp->length % 4096) required_page_count++;
  uintptr_t mapped_virtual_address = memory_map_physical_mmio(physical_address, required_page_count);
  ACPI_SDT_Header *rsdt = (ACPI_SDT_Header *)(mapped_virtual_address + page_offset);
  acpi_parse_root_system_descriptor_table(rsdt);
  return Error_Code_NONE;
}
