

internal void
video_initalize(VideoState *video, MultibootInfo *mbinfo) 
{
#if 0
	static const uint32_t VESA3_INFO_BLOCK_SIGNATURE = 
		(('A' << 24) | ('S' << 16) | ('E' << 8) | ('V'));
	static const uint32_t VESA3_PROTECTED_INTERFACE_SIGNATURE = 
		(('D' << 24) | ('I' << 16) | ('M' << 8) | ('P'));

	struct VESA3_InfoBlock {
		uint32_t signature;            // VBE Info Block signature ('VESA')
		uint16_t version;              // VBE version (0x300)
		uint32_t oem_string_ptr;       // OEM string
		uint8_t capabilities[4];       // capabilities of graphics controller
		uint32_t video_mode_ptr;       // video mode pointer
		uint16_t total_memory;         // # 64kb memory blocks
		uint16_t oem_software_rev;     // VBE implementation software revision
		uint32_t oem_vendor_name_ptr;  // vendor name
		uint32_t oem_product_name_ptr; // product name
		uint32_t oem_product_rev_ptr;  // product revision
		uint8_t _reserved[222];        // VBE implementation scratch area
		uint8_t oem_data[256];         // data area for OEM strings
	} __attribute__((packed));

	struct VESA3_ProtectedInterfaceBlock {
		uint32_t signature;     // PM Info Block signature ('PMID')
		uint32_t call_offs;     // offset of PM entry point within BIOS
		uint16_t init_offs;     // offset of PM initialization entry point
		uint16_t sel_bios_data; // selector to BIOS data area emulation block
		uint16_t sel_a0000;     // selector to 0xa0000
		uint16_t sel_b0000;     // selector to 0xb0000
		uint16_t sel_b8000;     // selector to 0xb8000
		uint16_t sel_code;      // selector to access code segment as data
		uint8_t in_pm;          // true iff in protected mode
		uint8_t checksum;       // sum of all bytes in this struct must match 0
	} __attribute((packed));

	VESA3_InfoBlock *vesa_info = (VESA3_InfoBlock *)mbinfo->vbe_control_info;
	uint8_t *vesa_info_signature = (uint8_t *)&vesa_info->signature;
	if (vesa_info_signature[0] == 'V' &&
			vesa_info_signature[1] == 'E' &&
			vesa_info_signature[2] == 'S' &&
			vesa_info_signature[3] == 'A') {
			klog("Multiboot provided a valid VESA Header");
			klog("VESA Version: %u", (uint32_t)vesa_info->version);

			if (vesa_info->signature == VESA3_INFO_BLOCK_SIGNATURE) {
				klog("secondary vesa sig formated correctly");
			}

	} else {
		klog("Multiboot vesa header is invalid");
	}

#endif

	struct VBE_ModeInfo {
		uint16_t attributes;
		uint8_t  winA, winB;
		uint16_t granularity;
		uint16_t winsize;
		uint16_t segmentA, segmentB;
		uint32_t realFctPtr;
		uint16_t pitch;

		uint16_t Xres, Yres;
		uint8_t  Wchar, Ychar, planes, bits_per_pixel, banks;
		uint8_t  memory_model, bank_size, image_pages;
		uint8_t  reserved0;

		uint8_t  red_mask, red_position;
		uint8_t  green_mask, green_position;
		uint8_t  blue_mask, blue_position;
		uint8_t  rsv_mask, rsv_position;
		uint8_t  directcolor_attributes;

		uint32_t physbase;
		uint32_t reserved1;
		uint16_t reserved2;
	} __attribute__ ((packed));

	VBE_ModeInfo *vbe_mode_info = (VBE_ModeInfo *)(uint64_t)mbinfo->vbe_mode_info;
	video->framebuffer = (uint8_t *)(uint64_t)vbe_mode_info->physbase;
	video->resolutionX = vbe_mode_info->Xres;
	video->resolutionY = vbe_mode_info->Yres;
	video->depth = vbe_mode_info->bits_per_pixel / 8;
}

void video_fill_rect(VideoState *video, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
	for (uint32_t yi = 0; yi < h; yi++) {
		uint8_t *write = video->framebuffer + (((y+yi) * video->resolutionX + x) * video->depth);
		for (uint32_t xi = 0; xi < w; xi++) {
			write[0] = 255;
			write[1] = 0;
			write[2] = 0;
			write[3] = 255;
			write += 4;
		}
	}

#if 0
	uint8_t *base = video->framebuffer + (((y * video->resolutionX) + x));
	for (uint32_t yi = 0; yi < h; yi++) {
		base += (video->resolutionX * 4) - (w * 4);
		for (uint32_t xi = 0; xi < w; xi++) {
			base[0] = 255;
			base[1] = 0;
			base[2] = 0;
			base[3] = 255;
			base += 4;
		}

	}

#endif
	
}



