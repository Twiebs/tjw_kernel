// Acronyms: 
// AHCI: Advance Host Controller Interface
// FIS: Frame Information Structure
// HBA: Host Bus Adapter
// ABAR: AHCI Base Memory Register

// Links: 
// https://wiki.osdev.org/AHCI
// https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf

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

typedef struct {
    union {
        struct {
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


static const uint32_t AHCI_PORT_SIGNATURE_SATA = 0x00000101;
static const uint32_t AHCI_PORT_SIGNATURE_SATAPI = 0xEB140101;
static const uint32_t AHCI_PORT_SIGNATURE_ENCLOSURE_MANAGEMENT_BRIDGE = 0xC33C0101;
static const uint32_t AHCI_PORT_SIGNATURE_PORT_MULTIPLIER = 0x96690101;

typedef struct {
    uint64_t command_list_base_address;
    uint64_t fis_base_address;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t command_and_status;
    uint32_t reserved0;
    uint32_t task_file_data;
    uint32_t signature;
    uint32_t sata_status;
    uint32_t sata_control;
    uint32_t sata_error;
    uint32_t sata_active;
    uint32_t command_issue;
    uint32_t sata_notification;
    uint32_t fis_switching_control;
    uint32_t device_sleep;
    uint32_t reserved1[10];
    uint32_t vendor_specific[4];
} __attribute((packed)) AHCI_Port_Registers;
static_assert(sizeof(AHCI_Port_Registers) == 128);

typedef struct {
    AHCI_HBA_Capabilities hba_capabilities;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t ports_implemented;
    uint16_t version_minor;
    uint16_t version_major;
    uint32_t command_completion_coalescing_control;
    uint32_t command_completion_coalescing_ports;
    uint32_t enclosure_management_location;
    uint32_t enclosure_management_control;
    uint32_t hba_capabilities_extended;
    uint32_t bios_os_handoff_control_and_status;

    uint8_t reserved[0xA0-0x2C];
    uint8_t vendor[0x100-0xA0];

    AHCI_Port_Registers ports[32];
} __attribute((packed)) AHCI_Registers;
static_assert(sizeof(AHCI_Registers) == (128 * 32) + 256);

Error_Code ahci_initalize(PCI_Device *pci_device);

