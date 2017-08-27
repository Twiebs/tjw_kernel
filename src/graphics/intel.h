
#include <stdint.h>

static const uint32_t INTEL_GMBUS0_PIN_PAIR_SELECT_DDC = 0b011;
static const uint32_t INTEL_GMBUS0_RATE_SELECT_100KHZ = 0b000;

static const uint32_t INTEL_GMBUS1_SLAVE_DIRECTION_WRITE = 0;
static const uint32_t INTEL_GMBUS1_SLAVE_DIRECTION_READ = 1;
//Intel_GMBUS1::bus_cycle_select
static const uint32_t INTEL_GMBUS1_BUS_CYCLE_SELECT_WAIT = 0b001;

//GMBUS0—GMBUS Clock/Port Select
typedef struct {
  volatile uint32_t pair_pin_select : 3;     //0-2
  volatile uint32_t reserved : 4;            //3-6
  volatile uint32_t hold_time_extension : 1; //7
  volatile uint32_t rate_select : 3;         //8-10
  volatile uint32_t reserved1 : 21;          //11-31
} __attribute((packed)) Intel_GMBUS0;

//GMBUS1—GMBUS Command/Status 
typedef struct {
  //1: read, 0: write
  volatile uint32_t slave_direction : 1;        //0
  volatile uint32_t slave_address : 7;          //1-7
  volatile uint32_t slave_register_index : 8;   //8-15
  volatile uint32_t total_byte_count : 9;       //16-24
  volatile uint32_t bus_cycle_select : 3;        //25-27
  volatile uint32_t reserved0 : 1;              //28
  volatile uint32_t enable_timeout : 1;         //29
  volatile uint32_t software_ready : 1;         //30
  volatile uint32_t software_clear_interrupt : 1;//31
} __attribute((packed)) Intel_GMBUS1;

//GMBUS2—GMBUS Status Registe
typedef struct {
  volatile uint32_t current_byte_count : 9; //0-8
  volatile uint32_t active : 1;             //9
  volatile uint32_t nak_inticator : 1;      //10
  volatile uint32_t hardware_ready : 1;     //11
  volatile uint32_t interrupt_status : 1;   //12
  volatile uint32_t slave_stall_timeout_error : 1; //13
  volatile uint32_t hardware_phase_wait : 1;    //14
  volatile uint32_t inuse : 1;              //15
  volatile uint32_t reserved : 16;          //16-31
} __attribute((packed)) Intel_GMBUS2;

//GMBUS3—GMBUS Data Buffer
typedef union {
  struct {
    volatile uint8_t data_byte_0;
    volatile uint8_t data_byte_1;
    volatile uint8_t data_byte_2;
    volatile uint8_t data_byte_3;
  };
  
  volatile uint32_t data;
} __attribute((packed)) Intel_GMBUS3;

//GMBUS4—GMBUS Interrupt Mask
typedef struct {
  volatile uint32_t interrupt_mask : 5;
  volatile uint32_t reserved0 : 27;
} __attribute((packed)) Intel_GMBUS4;

//GMBUS5—2 Byte Index Register
typedef struct {
  volatile uint32_t slave_index : 16;
  volatile uint32_t reserved0 : 15;
  volatile uint32_t two_byte_index_enable : 1;
} __attribute((packed)) Intel_GMBUS5;

static_assert(sizeof(Intel_GMBUS0) == 4);
static_assert(sizeof(Intel_GMBUS1) == 4);
static_assert(sizeof(Intel_GMBUS2) == 4);
static_assert(sizeof(Intel_GMBUS3) == 4);
static_assert(sizeof(Intel_GMBUS4) == 4);
static_assert(sizeof(Intel_GMBUS5) == 4);

typedef struct {
  volatile Intel_GMBUS0 *gmbus0;
  volatile Intel_GMBUS1 *gmbus1;
  volatile Intel_GMBUS2 *gmbus2;
  volatile Intel_GMBUS3 *gmbus3;
  volatile Intel_GMBUS4 *gmbus4;
  volatile Intel_GMBUS5 *gmbus5;
} Intel_Graphics_Device;

Error_Code intel_graphics_device_initalize(uintptr_t base_physical_address);
//Procedures for writing/reading over the DDC protocol on the I2C bus
//This is done via the Intel GMBUS registers
Error_Code intel_graphics_device_gmbus_wait_hardware_ready(Intel_Graphics_Device *graphics_device);
Error_Code intel_graphics_device_gmbus_wait_hardware_complete(Intel_Graphics_Device *graphics_device);
Error_Code intel_graphics_device_i2c_write(Intel_Graphics_Device *graphics_device, uint16_t address, uint64_t size, void *data);
Error_Code intel_graphics_device_i2c_read(Intel_Graphics_Device *graphics_device, uint16_t address, uint64_t size, void *buffer);
//Disable/Enable Digital To Analog Converter Connector IE. VGA Connectors
void intel_graphics_device_dac_enable(Intel_Graphics_Device *graphics_device);
void intel_graphics_device_dac_disable(Intel_Graphics_Device *graphics_device);