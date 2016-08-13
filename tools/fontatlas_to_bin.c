#if 0
clang -lm fontatlas_to_bin.c
exit
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
  int width, height, comp;
  uint8_t *data = stbi_load("bitmapfont.tga", &width, &height, &comp, 1);

  uint8_t *output = (uint8_t *)malloc(width * height); 

  for(size_t y0 = 0; y0 < 16; y0++){
    for(size_t x0 = 0; x0 < 6; x0++){

      for(size_t y1 = 0; y1 < 16; y1++){
        for(size_t x1 = 0; x1 < 16; x1++){
          size_t image_index = ((x0*16)+x1) + (((y0*16)+y1)*256);
          size_t buffer_index = (256*x0) + (y0*16*256);
           
        }
      } 



    }
  } 
    

}