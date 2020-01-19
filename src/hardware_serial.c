// https://wiki.osdev.org/Kernel_Debugging
// https://wiki.osdev.org/Serial_Ports

#define PORT 0x3F8
void serial_debug_init() 
{
   write_port_uint8(PORT + 1, 0x00);    // Disable all interrupts
   write_port_uint8(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   write_port_uint8(PORT + 0, 0x01);    // Set divisor to 3 (lo byte) 38400 baud
   write_port_uint8(PORT + 1, 0x00);    //                  (hi byte)
   write_port_uint8(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   write_port_uint8(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   write_port_uint8(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty() {
   return read_port_uint8(PORT + 5) & 0x20;
}
 
void write_serial(const char *src, size_t length){
  for(size_t i = 0; i < length; i++){
    while(is_transmit_empty() == 0) {}
    write_port_uint8(PORT, src[i]);
  }
}