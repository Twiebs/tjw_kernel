

typedef struct
{
    union {
        struct
        {
            uint32_t number_of_ports : 4;                         // 0-4
            uint32_t supports_external_sata : 1;                  // 5
            uint32_t enclosure_management_supported : 1;          // 6
            uint32_t command_completion_coalescing_supported : 1; // 7
            uint32_t number_of_command_slots : 4;                 // 8 –12
        };

        uint32_t value;
    };

} __attribute((packed)) AHCI_HBA_Capabilities;

static_assert(sizeof(AHCI_HBA_Capabilities) == 4);

Error_Code ahci_initalize(PCI_Device *pci_device);
