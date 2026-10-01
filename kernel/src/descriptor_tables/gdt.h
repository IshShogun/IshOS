#include "stdint.h"

#define GDT_SIZE 6

enum gdt_descriptor_flags {
	//only granularity i need
	GDT_LONG_MODE = 32U, //DB clear when this set. wont go to 32/16bit
	
	//access bytes
	GDT_PRESENT = 128U,
	GDT_USER = 96U,
	GDT_KERNEL = 0U,
	GDT_IS_NOT_SYSTEM = 16U, //code or data if set, ow is system ie TSS/LDT
	GDT_EXECUTABLE = 8U,
	GDT_RW = 2U
};

//in 64 bit, this is a descriptor table. describing exactly permissions for segments of memory. just no virtualisation base/limit
//32 bit was segment of memory (actual range) + permissions
typedef struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	//cpu checks these flags on segment access first, clear why. see sdm
	uint8_t access; // P | DPL | S | Type
	//more details
	uint8_t granularity; // G | D/B | L | AVL | limit_high[19:16]
	uint8_t base_high;
} gdt_entry_t; 

typedef struct gdt_pointer {
	uint64_t base;
	uint16_t limit;
} gdt_pointer_t;

void init_gdt();

void gdt_set_gate(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
