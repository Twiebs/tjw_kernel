#include <stdint.h>
#include <stdio.h>

static void
write_8bits_base16(char *dest, uint8_t value) {
	uint8_t top = (value >> 4);
        uint8_t bottom = ((uint8_t)(value << 4) >> 4);

#define write_four_bits(x)                                                     \
  if (x > 9)                                                                   \
    *dest = 'A' + (x - 10);                                                    \
  else                                                                         \
    *dest = '0' + x;

	write_four_bits(top);
	dest++;
	write_four_bits(bottom);
  #undef write_four_bits
}

static void write_uint8_t_base16(char *dest, uint8_t value) {
	dest[0] = '0';
	dest[1] = 'x';
	write_8bits_base16(dest + 2, value);
}

int main() {
	char buffer[5];
	write_uint8_t_base16(buffer, 0xFF);
	buffer[4] = 0;

	printf("%s\n", buffer);
	write_uint8_t_base16(buffer, 0x00);
	buffer[4] = 0;
	printf("%s\n", buffer);
	write_uint8_t_base16(buffer, 0x2D);
	buffer[4] = 0;
	printf("%s\n", buffer);
	write_uint8_t_base16(buffer, 0x11);
	buffer[4] = 0;
	printf("%s\n", buffer);
	write_uint8_t_base16(buffer, 0xCC);
	buffer[4] = 0;
	printf("%s\n", buffer);
	write_uint8_t_base16(buffer, 0xAB);
	buffer[4] = 0;
	printf("%s\n", buffer);





	return 0;
}


