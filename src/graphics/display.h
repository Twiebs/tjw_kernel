
static const uint64_t EDID_SIGNATURE = 0x00FFFFFF00;

typedef struct {
  uint8_t red_green_low_bits;
  uint8_t blue_white_low_bits;
  uint8_t red_x;
  uint8_t red_y;
  uint8_t green_x;
  uint8_t green_y;
  uint8_t blue_x;
  uint8_t blue_y;
  uint8_t white_x;
  uint8_t white_y;
} __attribute((packed)) Chroma_Information;

typedef struct {
  uint8_t unused : 1;         //0
  uint8_t sync_location : 1;  //1
  uint8_t serrate : 1;        //2
  uint8_t sync_type : 2;      //3-4
  uint8_t streo_mode : 2;      //5-6
  uint8_t interlaced : 1;      //7
} __attribute((packed)) Display_Type;

typedef struct {
  uint16_t pixel_clock; //10kHz * pixel_clock

  uint8_t horizontal_active_pixels_lsb;
  uint8_t horizontal_blanking_pixels_lsb;
  uint8_t horizontal_blanking_pixels_msb : 4;
  uint8_t horizontal_active_pixels_msb : 4;

  uint8_t vertical_active_lines_lsb;
  uint8_t vertical_blanking_lines_lsb;
  uint8_t vertical_blanking_lines_msb : 4;
  uint8_t vertical_active_lines_msb : 4;

  uint8_t horizontal_sync_offset_pixels_lsb;
  uint8_t horizontal_sync_pulse_width_pixels_lsb;
  uint8_t vertical_sync_pulse_width_lines_lsb : 4;
  uint8_t vertical_sync_offset_lines_lsb : 4;
  uint8_t vertical_sync_pulse_width_lines_msb : 2;
  uint8_t vertical_sync_offset_lines_msb : 2;
  uint8_t horizontal_sync_pulse_width_pixels_msb : 2;
  uint8_t horizontal_sync_offset_pixels_msb : 2;

  uint8_t horizontal_display_size_mm_lsb;
  uint8_t vertical_display_size_mm_lsb;
  uint8_t horizontal_display_size_mm_msb : 4;
  uint8_t vertical_display_size_mm_msb : 4;

  uint8_t horizontal_border_pixels; //each side; total is twice this
  uint8_t vertical_border_lines; //each side; total is twice this

  uint8_t features_bitmap;
} __attribute((packed)) Detailed_Timing_Description;

typedef struct {
  uint8_t resolution;
  uint8_t frequency;
} __attribute((packed)) Standard_Timing_Description;

typedef struct {
  uint64_t signature;
  uint16_t manufacture_id;
  uint16_t product_id;
  uint32_t serial_number;
  uint8_t manufacture_week;
  uint8_t manufacture_year;
  uint8_t version;
  uint8_t revision;
  //Basic Display Parameters / Features
  uint8_t video_input_type;
  uint8_t max_horizontal_size; //cm
  uint8_t max_vertical_size; //cm 
  uint8_t gama_factor;
  uint8_t dpms_flags;
  Chroma_Information chroma_information;
  uint8_t established_timings1;
  uint8_t established_timings2;
  uint8_t manufactures_reserved_timings;
  Standard_Timing_Description standard_timings[8];
  Detailed_Timing_Description detailed_timings[4];
  uint8_t extention_count;
  uint8_t checksum;
} __attribute((packed)) Extended_Display_Identification;

static_assert(sizeof(Chroma_Information) == 10);
static_assert(sizeof(Detailed_Timing_Description) == 18);
static_assert(sizeof(Extended_Display_Identification) == 128);

typedef struct {
  uint32_t pixel_clock; //kHz
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
} Display_Mode;


Error_Code extract_display_mode_information(Extended_Display_Identification *edid, Display_Mode *mode);
void convert_detailed_timing_descriptor_to_display_mode(Detailed_Timing_Description *dtd, Display_Mode *mode);