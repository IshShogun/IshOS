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

#define IDT_SIZE 255
//in here i should put the stub for the ISR handler -> trampoline to it
idt_entry_t idt[IDT_SIZE];

typedef void (*isr_t)(void);

//real isr handlers
isr_t isr_handlers[IDT_SIZE];


//just int -> isr, either switch stacks and calll, or call where you are. either way context is pushed for proper iretq.

//i just need isrs for cpu defined interrupts atm. a simple frame work of int -> debug statement -> isr_call 
//default isr just prints isr not set
//think i can do this with a macro

void handle_isr(int interrupt_id){
	isr_t isr = isr_handlers[interrupt_id];	
	isr();
}

void init_isrs(){

}

//how to handle ists?
void idt_set_gate(int8_t idx, int64_t isr, int16_t cs_selector, int8_t access){
	idt[idx].isr_low = (uint16_t) isr & 0x0F;
	idt[idx].isr_mid = (uint16_t) (isr >> 16) & 0x0F;
	idt[idx].isr_high = (uint32_t) isr >> 32;
	
	idt[idx].access = access;
}

void init_idt(){
	for(int idx = 0; idx < IDT_SIZE; idx++){
		idt[idx] = (struct idt_entry){
			.isr_low = 0,
			.cs_selector = 0,
			.ist = 0,
			.access = 0,
			.isr_mid = 0,
			.isr_high = 0,
			.reserved = 0,
		};
	}

}


