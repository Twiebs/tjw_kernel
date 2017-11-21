#if 0
clang png_to_binary_txt.c -lm -o ../bin/png_to_binary_txt
exit
#endif

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char **argv) {
  if(argc < 2) {
    printf("must provide input filename\n");
    return 1;
  } else if (argc < 3) {
    printf("must provide output filename\n");
    return 1;
  } else if (argc > 4) {
    printf("to many arguments\n");
    return 1;
  }

  const char *filename = argv[1];
  int width, height, components;
  uint8_t *pixel_data = stbi_load(filename, &width, &height, &components, 3);
  size_t image_size = width * height * components;


  size_t result_buffer_size = image_size * 5;
  uint8_t *result_buffer = (uint8_t *)malloc(result_buffer_size);
  for(size_t i = 0; i < image_size; i++){
    uint8_t value = pixel_data[i];
    sprintf((char*)result_buffer + (i*5), "0x%02X,", (uint32_t)value); 
  }

  {
    const char *output_filename = argv[2];
    FILE *file = fopen(output_filename, "w");
    fwrite(result_buffer, 1, result_buffer_size - 1, file); //removes trailing comma
    fclose(file);
  }

  return 0;
}