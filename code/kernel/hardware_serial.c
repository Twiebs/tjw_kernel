// https://wiki.osdev.org/Kernel_Debugging
// https://wiki.osdev.org/Serial_Ports

#include <stdint.h>

// NOTE(Torin, 2020-01-19) osdev(https://wiki.osdev.org/Serial_Ports) indicates that the port number for the serial ports can vary depending on
// how they are connected to the machine and how the BIOS is configured. COM1 and COM2 should almost always be
// these constants but COM3 and COM4 can vary more. I'm not sure how much of an issue this is in practice and how
// true this is in 2020. I'm assuming in 2020 every modern machine you would care about running on will be these values.
// That may not be true. I have not checked. Currently, the serial ports are only being used for debugging purposes.
// Emulators like bochs and qemu will write anything sent over the serial port out to a file. This is useful for logging.

static const uint16_t SERIAL_PORT_COM1 = 0x3F8;
static const uint16_t SERIAL_PORT_COM2 = 0x2F8;
static const uint16_t SERIAL_PORT_COM3 = 0x3E8;
static const uint16_t SERIAL_PORT_COM4 = 0x2E8;

static const uint16_t SERIAL_PORT_DATA_REGISTER_OFFSET = 0;
static const uint16_t SERIAL_PORT_INTERRUPT_ENABLE_REGISTER_OFFSET = 1;
static const uint16_t SERIAL_PORT_FIFO_CONTROL_REGISTER_OFFSET = 2;
static const uint16_t SERIAL_PORT_LINE_CONTROL_REGISTER_OFFSET = 3;
static const uint16_t SERIAL_PORT_MODEM_CONTROL_REGISTER_OFFSET = 4;

// Possible values For Serial_Port_FIFO_Control_Register::interrupt_trigger_level
static const uint8_t SERIAL_PORT_FIFO_CONTROL_REGISTER_INTERRUPT_TRIGGER_LEVEL_1 = 0b00;
static const uint8_t SERIAL_PORT_FIFO_CONTROL_REGISTER_INTERRUPT_TRIGGER_LEVEL_4 = 0b01;
static const uint8_t SERIAL_PORT_FIFO_CONTROL_REGISTER_INTERRUPT_TRIGGER_LEVEL_8 = 0b10;
static const uint8_t SERIAL_PORT_FIFO_CONTROL_REGISTER_INTERRUPT_TRIGGER_LEVEL_14 = 0b11;

// Possible values For Serial_Port_Line_Control_Register::word_length
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_WORD_LENGTH_5 = 0;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_WORD_LENGTH_6 = 1;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_WORD_LENGTH_7 = 2;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_WORD_LENGTH_8 = 3;

// Possible values For Serial_Port_Line_Control_Register::stop_bit_length
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_STOP_BIT_LENGTH_1 = 0;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_STOP_BIT_LENGTH_2 = 1;
// NOTE(Torin, 2020-01-19) If the word_length is 5 bits then the stop bit length
// will be set to 1.5 instead of 2 when the stop_bit_length bit is set to 1.
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_STOP_BIT_LENGTH_1_5 = 1;

// Possible values For Serial_Port_Line_Control_Register::parity_select
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_NO_PARITY = 0;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_ODD_PARITY = 1;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_EVEN_PARITY = 3;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_HIGH_PARITY_STICKY = 5;
static const uint8_t SERIAL_PORT_LINE_CONTROL_REGISTER_LOW_PARITY_STICKY = 7;

// https://www.lammertbies.nl/comm/info/serial-uart#FCR
typedef struct Serial_Port_FIFO_Control_Register 
{
    union 
    {
        struct 
        {
            volatile uint8_t fifo_enable                : 1; // 0
            volatile uint8_t clear_recieve_fifo         : 1; // 1
            volatile uint8_t clear_transmit_fifo        : 1; // 2
            volatile uint8_t dma_mode                   : 1; // 3
            volatile uint8_t reserved                   : 1; // 4
            volatile uint8_t enable_64byte_fifio        : 1; // 5
            volatile uint8_t interrupt_trigger_level    : 2; // 6-7
        };

        volatile uint8_t value; // 0-7
    };

} __attribute((packed)) Serial_Port_FIFO_Control_Register;

// https://www.lookrs232.com/rs232/lcr.htm
typedef struct Serial_Port_Line_Control_Register 
{
    union 
    {
        struct 
        {
            volatile uint8_t word_length          : 2; // 0-1
            volatile uint8_t stop_bit_length      : 1; // 2
            volatile uint8_t parity_select        : 3; // 3-5
            volatile uint8_t break_enable         : 1; // 6
            volatile uint8_t divisor_latch_access : 1; //7
        };

        volatile uint8_t value; // 0-7
    };

} __attribute((packed)) Serial_Port_Line_Control_Register;

// https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming
typedef struct Serial_Port_Modem_Control_Register 
{
    union 
    {
        struct 
        {
            volatile uint8_t data_terminal_ready          : 1; // 0
            volatile uint8_t request_to_send              : 1; // 1
            volatile uint8_t auxiliary_output_1           : 1; // 2
            volatile uint8_t auxiliary_output_2           : 1; // 3
            volatile uint8_t lookback_mode                : 1; // 4
            volatile uint8_t autoflow_control_enabled     : 1; // 5
            volatile uint8_t reserved0                    : 1; // 6
            volatile uint8_t reserved1                    : 1; // 7
        };

        volatile uint8_t value; // 0-7
    };

} __attribute((packed)) Serial_Port_Modem_Control_Register;

static_assert(sizeof(Serial_Port_FIFO_Control_Register) == 1);
static_assert(sizeof(Serial_Port_Line_Control_Register) == 1);
static_assert(sizeof(Serial_Port_Modem_Control_Register) == 1);

// NOTE(Torin, 2020-01-19) this function is intended to only be used in development builds
// for logging out to a file when running in An emulator such as bochs or qemu
// I currently do not have a macro for development builds so even though this function
// is prefixed with development_only It will always get compiled in. I'm not sure how I want to handle
// this yet, but I wanted to experiment with using the development_only prefix To see if it improves readability in the project.
void development_only_initialize_serial_port_logging()
{
    {
        // TODO(Torin, 2020-01-22) make sure the compiler considers line_control_register.value
        // Constant instead of actually allocating stack memory for it. Serial_Port_Line_Control_Register Has its fields
        // marked as volatile. I'm assuming this breaks any kind of constant folding optimization.

        // Turn on the divisor latch access so that we can set the baud rate divisor
        Serial_Port_Line_Control_Register line_control_register = {};
        line_control_register.divisor_latch_access = 1;
        write_port_uint8(SERIAL_PORT_COM1 + SERIAL_PORT_LINE_CONTROL_REGISTER_OFFSET, line_control_register.value);
    }

    // After the line control register has the divisor_latch_access Bit set:
    // Register offset 0 now becomes the least significant byte of the baud rate divisor
    // Register offset 1 now becomes the most significant byte of the baud rate divisor

    // Set the baud rate to 38400 by setting the divisor to 3
    // NOTE(Torin, 2020-01-19) for some reason in the old version of this function I was setting the divisor
    // to 1 instead of 3. I've absolutely no idea why was doing that. The old function was a verbatim copy of
    // the example initialization code found at https://wiki.osdev.org/Serial_Ports. There was still a comment
    // Saying that it was sending the divisor to three even though the value is one. I'm setting it back to three
    // And will leave it that way As long as there are no issues.

    write_port_uint8(SERIAL_PORT_COM1 + 0, 0x03);
    write_port_uint8(SERIAL_PORT_COM1 + 1, 0x00);

    {
        // Turn off divisor latch access, set word length, stop bit length, parity.
        Serial_Port_Line_Control_Register line_control_register = {};
        line_control_register.word_length = SERIAL_PORT_LINE_CONTROL_REGISTER_WORD_LENGTH_8;
        line_control_register.stop_bit_length = SERIAL_PORT_LINE_CONTROL_REGISTER_STOP_BIT_LENGTH_1;
        line_control_register.parity_select = SERIAL_PORT_LINE_CONTROL_REGISTER_NO_PARITY;
        line_control_register.divisor_latch_access = 0;
        write_port_uint8(SERIAL_PORT_COM1 + SERIAL_PORT_LINE_CONTROL_REGISTER_OFFSET, line_control_register.value);
    }

    {
        Serial_Port_FIFO_Control_Register fifo_control_register = {};
        fifo_control_register.fifo_enable = 1;
        fifo_control_register.clear_recieve_fifo = 1;
        fifo_control_register.clear_transmit_fifo = 1;
        fifo_control_register.interrupt_trigger_level = SERIAL_PORT_FIFO_CONTROL_REGISTER_INTERRUPT_TRIGGER_LEVEL_14;
        write_port_uint8(SERIAL_PORT_COM1 + SERIAL_PORT_FIFO_CONTROL_REGISTER_OFFSET, fifo_control_register.value);
    }

    {
        Serial_Port_Modem_Control_Register modem_control_register = {};
        modem_control_register.data_terminal_ready = 1;
        modem_control_register.request_to_send = 1;
        modem_control_register.auxiliary_output_2 = 1;
        write_port_uint8(SERIAL_PORT_COM1 + SERIAL_PORT_MODEM_CONTROL_REGISTER_OFFSET, modem_control_register.value);
    }
}

int is_transmit_empty() {
   return read_port_uint8(SERIAL_PORT_COM1 + 5) & 0x20;
}
 
void write_serial(const char *src, size_t length){
  for(size_t i = 0; i < length; i++){
    while(is_transmit_empty() == 0) {}
    write_port_uint8(SERIAL_PORT_COM1, src[i]);
  }
}