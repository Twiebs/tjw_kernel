#if 0
clang -lm fontatlas_to_bin.c
exit
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static inline
void bitmap_to_linear_data(uint8_t *result, uint32_t xorigin, uint32_t yorigin, uint32_t stride, uint8_t *buffer){
  size_t write_index = 0;
  for(size_t y = yorigin; y < (yorigin+16); y++){
    for(size_t x = xorigin; x < (xorigin+16); x++){
      size_t index = x + y*stride; 
      result[write_index] = buffer[index];
      write_index++;
    }
  }
} 

uint8_t *uint8_buffer_to_comma_seperated_string(uint8_t *buffer, size_t buffer_size, size_t *result_size){
  *result_size = buffer_size * 5;
  uint8_t *result_buffer = (uint8_t *)malloc(*result_size);
  for(size_t i = 0; i < buffer_size; i++){
    uint8_t value = buffer[i];
    sprintf((char*)result_buffer + (i*5), "0x%02X,", (uint32_t)value); 
  }
  return result_buffer;
}

int WriteDataToFile(const char *filename, uint8_t *data, size_t size){
  FILE *file = fopen(filename, "wb");
  if(file == NULL) return 0;
  fwrite(data, size, 1, file);
  fclose(file);
  return 1;
}

int main() {
  int width, height, comp;
  uint8_t *data = stbi_load("bitmapfont.tga", &width, &height, &comp, 1);
  uint8_t *output = (uint8_t *)malloc(width * height); 


  size_t write_index = 0;
  for(size_t y = 0; y < 6; y++){
    for(size_t x = 0; x < 16; x++){
      uint32_t xorigin = x*16;
      uint32_t yorigin = y*16;
      bitmap_to_linear_data(output + write_index, xorigin, yorigin, width, data);
      write_index += 256;
    }
  }


  size_t result_size = 0;
  uint8_t *result = uint8_buffer_to_comma_seperated_string(output, width*height, &result_size); 
  WriteDataToFile("bitmapfont.txt", result, result_size);


    

}