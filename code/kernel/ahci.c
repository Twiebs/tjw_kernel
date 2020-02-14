
void debug_log_AHCI_HBA_Capabilities(volatile AHCI_HBA_Capabilities *hba_capabilities)
{
    log_debug(AHCI, "hba_capabilities:");
    log_debug(AHCI, "  number_of_ports: %u", (uint32_t)hba_capabilities->number_of_ports);
    log_debug(AHCI, "  supports_external_sata: %u", (uint32_t)hba_capabilities->supports_external_sata);
    log_debug(AHCI, "  enclosure_management_supported: %u", (uint32_t)hba_capabilities->enclosure_management_supported);
    log_debug(AHCI, "  command_completion_coalescing_supported: %u", (uint32_t)hba_capabilities->command_completion_coalescing_supported);
    log_debug(AHCI, "  number_of_command_slots: %u", (uint32_t)hba_capabilities->number_of_command_slots);
}

uint16_t ahci_decode_minor_version_number(const uint16_t minor_version_number)
{
    if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_95)
    {
        return 95;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_0)
    {
        return 0;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_1)
    {
        return 1;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_2)
    {
        return 2;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_3)
    {
        return 3;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_3_1)
    {
        return 3;
    }

    return 0xFFFF;
}

uint16_t ahci_decode_revision_version_number(const uint16_t minor_version_number)
{
    if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_95)
    {
        return 0;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_0)
    {
        return 0;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_1)
    {
        return 0;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_2)
    {
        return 0;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_3)
    {
        return 0;
    }
    else if (minor_version_number == AHCI_MINOR_VERSION_NUMBER_3_1)
    {
        return 1;
    }

    return 0xFFFF;
}

// NOTE(Torin, 2020-02-13) the AHCI Only specifies a very small number of supported version Numbers.
// If the device reports a version number that is not in the specification we will not attempt to initialize it.
bool ahci_is_version_supported(const uint16_t major_version_number, const uint16_t minor_version_number, const uint16_t revision_version_number)
{
    if (major_version_number == 0 && minor_version_number == 95 && revision_version_number == 0)
    {
        return true;
    }
    else if (major_version_number == 1 && minor_version_number == 0 && revision_version_number == 0)
    {
        return true;
    }
    else if (major_version_number == 1 && minor_version_number == 1 && revision_version_number == 0)
    {
        return true;
    }
    else if (major_version_number == 1 && minor_version_number == 2 && revision_version_number == 0)
    {
        return true;
    }
    else if (major_version_number == 1 && minor_version_number == 3 && revision_version_number == 0)
    {
        return true;
    }
    else if (major_version_number == 1 && minor_version_number == 3 && revision_version_number == 1)
    {
        return true;
    }

    return false;
}

Error_Code ahci_initalize(PCI_Device *pci_device)
{
    log_info(AHCI, "Initializing AHCI - Advanced Host Controller Interface (SATA)");

    pci_debug_log_pci_device(pci_device);

    const uintptr_t registers_physical_address =  pci_device->base_address_5;
    log_info(AHCI, "Initializing AHCI at physical address 0x%X", registers_physical_address);

    const Virtual_Address registers_virtual_address = memory_map_physical_mmio(registers_physical_address, 1);

    volatile AHCI_Registers *registers = (volatile AHCI_Registers *)registers_virtual_address;

    // NOTE(Torin, 2020-02-13) the hardware does not actually specify the versions in this way.
    // There is some kind of weird encoding in the minor version number That I'm not sure with the purpose of is.
    // I want to be able to log out what the version number actually is so I am adding this decode step.
    // These are the numbers that will be used to figure out if the current version is supported.
    const uint16_t major_version_number  = registers->version_major;
    const uint16_t minor_version_number = ahci_decode_minor_version_number(registers->version_minor);
    const uint16_t revision_version_number = ahci_decode_revision_version_number(registers->version_minor);
    if (!ahci_is_version_supported(major_version_number, minor_version_number, revision_version_number))
    {
        log_error(AHCI, "Unsupported AHCI Version: %u.%u.%u. The device will not be initialized.", (uint32_t)major_version_number, (uint32_t)minor_version_number, (uint32_t)revision_version_number);
        return Error_Code_UNSUPORTED_FEATURE;
    }

    log_debug(AHCI, "Supported AHCI Version: %u.%u.%u", (uint32_t)major_version_number, (uint32_t)minor_version_number, (uint32_t)revision_version_number);
    volatile AHCI_HBA_Capabilities *hba_capabilities = &registers->hba_capabilities;
    debug_log_AHCI_HBA_Capabilities(hba_capabilities);



    return Error_Code_NONE;
}