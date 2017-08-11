
//TODO(Torin: 2017-08-11) Exceptionally hacky 
uint8_t *g_temporary_page;

uint8_t *cpu_get_temporary_page() {
  if (g_temporary_page == 0) {

  }
  return g_temporary_page;
}