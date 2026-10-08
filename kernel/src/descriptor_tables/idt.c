#include <stdint.h>
#include <stdio.h>
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "interrupt.h"

//just int -> isr, either switch stacks and calll, or call where you are. either way context is pushed for proper iretq.

//i just need isrs for cpu defined interrupts atm. a simple frame work of int -> debug statement -> isr_call 
//default isr just prints isr not set
//think i can do this with a macro

idtr_t idt_pointer;
extern void init_isrs();
extern void idt_flush(uint64_t ptr);
idt_entry_t idt[IDT_SIZE];

void idt_set_gate(uint8_t idx, uint64_t isr_stub_addr, uint8_t access, uint16_t seg_selector, uint8_t ist)
{
    idt[idx].isr_low     = (uint16_t)(isr_stub_addr & 0xFFFF);
    idt[idx].isr_mid     = (uint16_t)((isr_stub_addr >> 16) & 0xFFFF);
    idt[idx].isr_high    = (uint32_t)(isr_stub_addr >> 32);
    idt[idx].cs_selector = seg_selector;
    idt[idx].ist         = ist & 0x7;
    idt[idx].access      = access;
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

	init_isrs();

	idt_set_gate(INT_DE,  (uint64_t)&ISR_0,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_DB,  (uint64_t)&ISR_1,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_NMI, (uint64_t)&ISR_2,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NMI);
	idt_set_gate(INT_BP,  (uint64_t)&ISR_3,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_OF,  (uint64_t)&ISR_4,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_BR,  (uint64_t)&ISR_5,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_UD,  (uint64_t)&ISR_6,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_NM,  (uint64_t)&ISR_7,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_DF,  (uint64_t)&ISR_ERR_8,  GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_DF);
	idt_set_gate(INT_CSO, (uint64_t)&ISR_9,      GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_TS,  (uint64_t)&ISR_ERR_10, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_NP,  (uint64_t)&ISR_ERR_11, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_SS,  (uint64_t)&ISR_ERR_12, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_GP,  (uint64_t)&ISR_ERR_13, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_PF,  (uint64_t)&ISR_ERR_14, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_MF,  (uint64_t)&ISR_16,     GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_AC,  (uint64_t)&ISR_ERR_17, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_MC,  (uint64_t)&ISR_18,     GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_MC);
	idt_set_gate(INT_XM,  (uint64_t)&ISR_19,     GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_VE,  (uint64_t)&ISR_20,     GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);
	idt_set_gate(INT_CP,  (uint64_t)&ISR_ERR_21, GDT_PRESENT | GDT_KERNEL | GDT_IS_SYSTEM_INT_GATE, 0x08, IST_NONE);

	idt_pointer.base = (uint64_t) &idt;
	idt_pointer.limit = IDT_SIZE * sizeof(idt_entry_t) - 1;

	idt_flush((uint64_t) &idt_pointer);
}


