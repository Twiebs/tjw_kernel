
#include "../utils/dynamic_count_static_array.h"

typedef enum {
  Search_Entry_Type_INVALID,
  Search_Entry_Type_PROGRAM,
} Search_Entry_Type;

typedef struct {
  Search_Entry_Type type;
  //TODO(Torin, 2017-10-21) Better string handling for this struct
  //Also we should support searching for multiple different strings!
  char search_string[128];
  size_t search_string_length;
  //TODO(Torin, 2017-10-21) Description support
} Search_Entry;

DEFINE_DYNAMIC_COUNT_STATIC_ARRAY(Search_Entry_Array, search_entry_array, Search_Entry, 8)

typedef struct {
  bool is_overlay_active;
  Graphics_Device *graphics_device;
  Software_Graphics_Buffer desktop_buffer;
  Software_Graphics_Buffer overlay_buffer;

  Search_Entry_Array search_entry_array;
} Desktop_Enviroment;

void desktop_enviroment_initialize(Desktop_Enviroment *desktop_enviroment, Graphics_Device *graphics_device);
void desktop_enviroment_render(Desktop_Enviroment *desktop_enviroment);
void desktop_enviroment_process_input(Desktop_Enviroment *desktop_enviroment, Keyboard_State *keyboard_state);
void desktop_enviroment_update(Desktop_Enviroment *desktop_enviroment);

void desktop_enviroment_draw_overlay(Desktop_Enviroment *desktop_enviroment);