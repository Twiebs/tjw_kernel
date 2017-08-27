
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
  uint8_t horizontal_frequency; //kHz
  uint8_t vertical_frequency;   //kHz
  uint8_t horizontal_active_time;
  uint8_t horizontal_blanking_time;  
  uint8_t vertical_active_time;
  uint8_t vertical_blanking_time;
  uint8_t torin_is_lazy[12];
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