#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

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
  FILE *file = fopen(filename, "rb");
  if(file == 0) {
    printf("could not open input file: %s\n", filename);
    return 2;
  }
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);  
  uint8_t *file_data = (uint8_t *)malloc(file_size);
  fread(file_data, 1, file_size, file);  
  fclose(file);

  size_t result_buffer_size = file_size * 5;
  uint8_t *result_buffer = (uint8_t *)malloc(result_buffer_size);
  for(size_t i = 0; i < file_size; i++){
    uint8_t value = file_data[i];
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