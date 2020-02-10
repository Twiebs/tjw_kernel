

typedef struct
{
    union 
    {
        struct
        {
            uint32_t number_of_ports : 4; // 0-4
        };

        uint32_t value;
    };

} __attribute((packed)) AHCI_HBA_Capabilities;

static_assert(sizeof(AHCI_HBA_Capabilities) == 4);

Error_Code ahci_initalize(PCI_Device *pci_device);
