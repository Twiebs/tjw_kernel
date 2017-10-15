
typedef struct {
  bool is_overlay_active;
  Graphics_Device *graphics_device;
  Software_Graphics_Buffer desktop_buffer;
  Software_Graphics_Buffer overlay_buffer;
} Desktop_Enviroment;

void desktop_enviroment_initialize(Desktop_Enviroment *desktop_enviroment, Graphics_Device *graphics_device);
void desktop_enviroment_render(Desktop_Enviroment *desktop_enviroment);
void desktop_enviroment_process_input(Desktop_Enviroment *desktop_enviroment, Keyboard_State *keyboard_state);
void desktop_enviroment_update(Desktop_Enviroment *desktop_enviroment);

void desktop_enviroment_draw_overlay(Desktop_Enviroment *desktop_enviroment);