
struct VBEInfoBlock {
	const char vbe_signature[4];
	uint16_t vbe_version;
	uint16_t oem_string_ptr[2];
	uint8_t capabilities[4];
	uint16_t video_mode_ptr[2];
	uint16_t total_memory;
} __attribute((packed));

struct VesaModeInfo {
	uint16_t attributes;
	uint8_t win_a, win_b;
	uint16_t granularity;

	uint16_t winsize;
};
