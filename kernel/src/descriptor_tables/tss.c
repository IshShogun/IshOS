#include <stdint.h>
#include "tss.h"
#include "gdt.h"

tss_entry_t tss;

#define IST_STACK_SIZE 4096

static char NMI_STACK[IST_STACK_SIZE] __attribute__((aligned(16)));
static char DF_STACK[IST_STACK_SIZE] __attribute__((aligned(16)));
static char MCE_STACK[IST_STACK_SIZE] __attribute__((aligned(16)));

//You want one on #DF #NMI and #MCE (though I’m lazy and didn’t do it on #MCE yet )
//need rsp0, rsp3 (why not), and ists for df, nmi and mce. 1 ist each or point to same ist?
//4kb stack for the ists

void init_tss(int idx){
	gdt_set_gate(idx, (uint32_t) &tss, sizeof(tss_entry_t) - 1, GDT_IS_SYSTEM_TSS | GDT_PRESENT , 0x0);

	gdt[idx + 1].limit_low = (uint16_t) ((uint64_t) &tss >> 32);
	gdt[idx + 1].base_low = (uint16_t) ((uint64_t) &tss >> 48);

	//set tss.rsp0 when i have scheduling as threads will need their own kernel stack and user stack - only needed on change of cpl
	//but for now ill set df, nmi & mce
 //stack grows downards starts at high address ends at low
	tss.ist[IST_NMI - 1] = (uint64_t) NMI_STACK + IST_STACK_SIZE;
	tss.ist[IST_DF - 1] = (uint64_t) DF_STACK + IST_STACK_SIZE;
	tss.ist[IST_MC - 1] = (uint64_t) MCE_STACK + IST_STACK_SIZE;

	//all i need for now..
	return;
}
