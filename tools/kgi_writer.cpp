#define STB_IMAGE_IMPLEMENTATION

#include "../stb_image.h"
#include <stdint.h>
#include <stdio.h>

int main(int argc, char **argv) {
	if (argc < 2) { 
		printf("must provide filename\n");
		return 1;
	}

	int width, height, components;
	uint8_t *pixels = stbi_load(argv[1], &width, &height, &components, 4);
	if (components != 4) {
		printf("invalid component count");
	}

	static const uint32_t TKIF_VERIFIER = ('F' << 24) | ('I' << 16) | ('K' << 8) | ('T');

	uint32_t out_width = (uint32_t)width;
	uint32_t out_height = (uint32_t)height;
	uint32_t out_components = (uint32_t)components;

	FILE *file = fopen("out.kgi", "wb");
	fwrite(&TKIF_VERIFIER, sizeof(uint32_t), 1, file);
	fwrite(&out_width, sizeof(uint32_t), 1, file);
	fwrite(&out_height, sizeof(uint32_t), 1, file);
	fwrite(&out_components, sizeof(uint32_t), 1, file);
	fwrite(pixels, components * width * height, 1, file);
	fclose(file);

}
