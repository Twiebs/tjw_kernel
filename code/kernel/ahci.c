
void debug_log_AHCI_HBA_Capabilities(volatile AHCI_HBA_Capabilities *hba_capabilities)
{
    log_debug(AHCI, "hba_capabilities:");
    log_debug(AHCI, "  number_of_ports: %u", (uint32_t)hba_capabilities->number_of_ports);
    log_debug(AHCI, "  supports_external_sata: %u", (uint32_t)hba_capabilities->supports_external_sata);
    log_debug(AHCI, "  enclosure_management_supported: %u", (uint32_t)hba_capabilities->enclosure_management_supported);
    log_debug(AHCI, "  command_completion_coalescing_supported: %u", (uint32_t)hba_capabilities->command_completion_coalescing_supported);
    log_debug(AHCI, "  number_of_command_slots: %u", (uint32_t)hba_capabilities->number_of_command_slots);
}

Error_Code ahci_initalize(PCI_Device *pci_device)
{
    log_info(AHCI, "Initializing AHCI - Advanced Host Controller Interface (SATA)");

    const Physical_Address registers_physical_address =  pci_device_get_base_address_0(pci_device);
    const Virtual_Address registers_virtual_address = memory_map_physical_mmio(registers_physical_address, 1);

    volatile AHCI_HBA_Capabilities *hba_capabilities = (volatile AHCI_HBA_Capabilities *)registers_virtual_address;
    debug_log_AHCI_HBA_Capabilities(hba_capabilities);

    return Error_Code_NONE;
}