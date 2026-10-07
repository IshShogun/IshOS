#include <stdint.h>

typedef struct idt_entry   {
	uint16_t isr_low;
	uint16_t cs_selector;
	uint8_t ist;
	uint8_t access;
	uint16_t isr_mid;
	uint32_t isr_high;
	uint32_t reserved;
} __attribute__((packed)) idt_entry_t;


__attribute__((aligned(16)))
idt_entry_t idt[255];

void init_idt(){
	//df, mce, nmi	
}


