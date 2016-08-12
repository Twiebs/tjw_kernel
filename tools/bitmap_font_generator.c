#if 0
clang -g -O0 -lm bitmap_font_generator.c -o bitmap_font_generator
exit
#endif

#include <math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_truetype.h"
#include "stb_image_write.h"

#include <stdint.h>
#include <stdio.h>

uint8_t *ReadFileIntoMemory(const char *filename, size_t *outFileSize){
  FILE *file = fopen(filename, "rb");
  if(file == 0) return 0;
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  uint8_t *result = (uint8_t *)malloc(fileSize);
  fread(result, 1, fileSize, file);
  fclose(file);
  if(outFileSize != 0) *outFileSize = fileSize;
  return result;
}

int WriteDataToFile(const char *filename, uint8_t *data, size_t size){
  FILE *file = fopen(filename, "wb");
  if(file == NULL) return 0;
  fwrite(data, size, 1, file);
  fclose(file);
  return 1;
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


int main() {

  const char *ttf_filename = "inconsolata.ttf";
  size_t file_size = 0;
  uint8_t *ttf_data =  ReadFileIntoMemory(ttf_filename, &file_size);
  if(ttf_data == 0) {
    printf("could not find ttf file: %s\n", ttf_filename);
    return 1;
  }

  stbtt_fontinfo font;
  stbtt_InitFont(&font, ttf_data, 0);

  static const size_t buffer_width  = 256;
  static const size_t buffer_height = 96;
  uint8_t *buffer = (uint8_t *)malloc(buffer_width * buffer_height);
  memset(buffer, 0x00, buffer_width * buffer_height);

  size_t row_index = 0; size_t column_index = 0;
  int glyph_width = 0, glyph_height = 0;

  for(size_t i = ' ';  i <= '~'; i++){
    uint8_t *glyph_data = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 16), i, &glyph_width, &glyph_height, 0, 0);

    for(size_t y = 0; y < glyph_height; y++){
      for(size_t x = 0; x < glyph_width; x++){
        size_t glyph_index = x + y*glyph_width;
        size_t buffer_index = ((i - ' ')*256) + (x + y*16);
        buffer[buffer_index] = glyph_data[glyph_index];
      }
    }



    #if 0
    for(size_t y = 0; y < glyph_height; y++){
      for(size_t x = 0; x < glyph_width; x++){
        size_t glyph_index = x + y*glyph_width;
        size_t buffer_index = (x+(column_index*16)) + ((y+(row_index*16))*buffer_width); 
        buffer[buffer_index] = glyph_data[glyph_index]; 
      }
    }
    #endif

    column_index++;
    if(column_index >= 16){
      column_index = 0;
      row_index++;
    }
  }

  size_t text_size = 0;
  uint8_t *text_data = uint8_buffer_to_comma_seperated_string(buffer, buffer_width * buffer_height, &text_size);
  WriteDataToFile("inconsolata16.txt", text_data, text_size);

  stbi_write_png("font.png", buffer_width, buffer_height, 1, buffer, buffer_width);
  return 0;
}