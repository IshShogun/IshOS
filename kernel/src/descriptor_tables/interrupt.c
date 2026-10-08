#include <stdint.h>
#include "interrupt.h"
#include <stdio.h>
#include "idt.h"

isr_t isr_handlers[IDT_SIZE];
void handle_isr(interrupt_frame_t *context){
	isr_t isr = isr_handlers[context->interrupt_id];	
	//context is just a label, when i declare *context its just saying this label has an addy
	isr(context);
}

void default_isr(interrupt_frame_t *context){
	printf("INT OCCURED: %i", context->interrupt_id);
}

void init_isrs(){
	for(int i = 0; i < IDT_SIZE; i++){
		isr_handlers[i] = default_isr;
	}
}
