
// AHCI: Advance Host Controller Interface
// FIS: Frame Information Structure
// HBA: Host Bus Adapter

// NOTE(Torin, 2020-02-13) for some godforsaken reason the AHCI minor version number
// is not a regular integer. It has to be one of these values. At first glance it may look like
// the Most significant byte Is the minor version number in the least significant byte is the revision number
// but AHCI_MINOR_VERSION_NUMBER_95 Breaks this rule. I'm not sure what the fuck they were thinking.
// I feel like there is some reason for this but I can't figure it out and I don't see it documented anywhere.
static const uint16_t AHCI_MINOR_VERSION_NUMBER_95 = 0x0905;
static const uint16_t AHCI_MINOR_VERSION_NUMBER_0 = 0x0000;
static const uint16_t AHCI_MINOR_VERSION_NUMBER_1 = 0x0100;
static const uint16_t AHCI_MINOR_VERSION_NUMBER_2 = 0x0200;
static const uint16_t AHCI_MINOR_VERSION_NUMBER_3 = 0x0300;
static const uint16_t AHCI_MINOR_VERSION_NUMBER_3_1 = 0x0301;

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

typedef struct
{
    AHCI_HBA_Capabilities hba_capabilities;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t ports_implemented;
    uint16_t version_minor;
    uint16_t version_major;
} __attribute((packed)) AHCI_Registers;

static_assert(sizeof(AHCI_HBA_Capabilities) == 4);

Error_Code ahci_initalize(PCI_Device *pci_device);

