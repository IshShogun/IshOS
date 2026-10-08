#include "stdint.h"
#include "gdt.h"
#include "tss.h"

gdt_entry_t gdt[GDT_SIZE];

//says lgdt source operand specifies a 10 byte memory location, 8 for base, 2 for limit
extern void gdt_flush(uint64_t ptr);
//this is a region of memory that stores the base and the limit of the gdt. may want to be ref elsewhere. 
gdtr_t gdt_ptr;

void init_gdt(){
	for(uint8_t idx = 0; idx < GDT_SIZE; idx++){
		gdt[idx] = (struct gdt_entry) {
			.limit_low = 0,
			.base_low = 0,
			.base_middle = 0,
			.access = 0,
			.flags = 0,
			.base_high = 0,
		};
	};

	//null descriptor
	gdt_set_gate(0, 0, 0, 0, 0);
	//kernel mode code segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_KERNEL | GDT_IS_NOT_SYSTEM | GDT_EXECUTABLE | GDT_RW,  GDT_GRANULARITY | GDT_LONG_MODE);
	//kernel mode data segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_KERNEL | GDT_IS_NOT_SYSTEM | GDT_RW, GDT_GRANULARITY | GDT_DB);
	//user mode data segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_USER | GDT_IS_NOT_SYSTEM | GDT_RW, GDT_GRANULARITY | GDT_DB);
	//user mode code segment
	gdt_set_gate(4, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_USER | GDT_IS_NOT_SYSTEM | GDT_EXECUTABLE | GDT_RW, GDT_LONG_MODE | GDT_GRANULARITY);

	gdt_ptr.limit = GDT_SIZE * sizeof(gdt_entry_t) - 1;
	gdt_ptr.base = (uint64_t)&gdt;

	//i only need a tss when i have a userspace or i want seperate stacks for interrupt management.
	//guess ill need ISTs to make it preemptable? buut the point of rsp0 is to continue the execution 
	//of the user thread in kernel mode (serving a syscall). so it gets a fresh sstack to execuute (RSP0)
	//user thread -> is a thread that serves userspaace (executing in ring3 -> syscall -> fresh stack -> do stuff -> return to context) 
	//the scheduler will manage all these threads.
	init_tss(5);
	gdt_flush((uint64_t) &gdt_ptr);

	//TODO: tss - update to 6 for this
};

void gdt_set_gate(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
	if(index >= GDT_SIZE){
		//printf("ya fucked up, idx bigger than gdt size");
	}

	//base address - usually 0 in 64 bit, but needed for TSS
	gdt[index].base_low = base & 0xFFFFU;
	gdt[index].base_middle = (base >> 16) & 0xFFU;
	gdt[index].base_high = (base >> 24) & 0xFFU;

	//limit
	gdt[index].limit_low = limit & 0xFFFFU;
	gdt[index].base_middle = (base >> 16) & 0x0FU;

	//descriptor flags
	gdt[index].access = access;
	gdt[index].flags = flags;
};
