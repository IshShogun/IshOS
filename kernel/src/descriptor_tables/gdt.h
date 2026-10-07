#include "stdint.h"

#define GDT_SIZE 7


enum gdt_descriptor_flags {
	//flags
  //DB clear in long mode code segment. wont go to 32/16bit
	GDT_GRANULARITY = 128U,
	GDT_LONG_MODE = 32U, 
	//set by convention
	GDT_DB = 64U,
	
	//access bytes
	GDT_PRESENT = 128U,
	GDT_USER = 96U,
	GDT_KERNEL = 0U,
	GDT_IS_NOT_SYSTEM = 16U, //code or data if set, ow is system ie TSS/LDT
	GDT_EXECUTABLE = 8U,
	GDT_RW = 2U,

	//system descriptor types when system is clear - sdm 3.5
	GDT_IS_SYSTEM_TSS = 0x9
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
	uint8_t flags; // G | D/B | L | AVL | limit_high[19:16]
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t; 

typedef struct   gdt_pointer {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt_pointer_t;

gdt_entry_t gdt[GDT_SIZE];

void init_gdt();

void gdt_set_gate(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
