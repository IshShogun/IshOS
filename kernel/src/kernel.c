#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm_backends/fb.h>
#include <flanterm.h>
#include <descriptor_tables/gdt.h>
#include <descriptor_tables/idt.h>
#include <utils/limine.h>
#include <mem/paging.h>
#include <mem/alloc/buddy.h>
#include <panic.h>

void memmap_print(void) {
    unsigned long long usable_pre = 0;
    unsigned long long usable_post = 0;

    printf("Limine memory map:\n");
    printf("+-------+--------------------+--------------------+--------------------+------------------------+\n");
    printf("| Index | Phys Start         | Phys End           | Size               | Type                   |\n");
    printf("+-------+--------------------+--------------------+--------------------+------------------------+\n");

    for (unsigned long long i = 0; i < memmap_request.response->entry_count; i++) {
        unsigned long long base   = memmap_request.response->entries[i]->base;
        unsigned long long length = memmap_request.response->entries[i]->length;
        unsigned long long type   = memmap_request.response->entries[i]->type;

        const char *type_str;
        switch (type) {
            case LIMINE_MEMMAP_USABLE:
                type_str = "Usable";
                usable_pre  += length;
                usable_post += length;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                type_str = "Bootloader Reclaimable";
                usable_post += length;
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                type_str = "ACPI Reclaimable";
                usable_post += length;
                break;
            case LIMINE_MEMMAP_RESERVED:               type_str = "Reserved"; break;
            case LIMINE_MEMMAP_ACPI_NVS:               type_str = "ACPI NVS"; break;
            case LIMINE_MEMMAP_BAD_MEMORY:             type_str = "Bad Memory"; break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES: type_str = "Executable and Modules"; break;
            case LIMINE_MEMMAP_FRAMEBUFFER:            type_str = "Framebuffer"; break;
            case LIMINE_MEMMAP_RESERVED_MAPPED:        type_str = "Reserved Mapped"; break;
            default:                                   type_str = "Unknown"; break;
        }

        printf("| %5lu | 0x%016lx | 0x%016lx | 0x%016lx | %s |\n",
               i, base, base + length, length, type_str);
    }

    printf("+-------+--------------------+--------------------+--------------------+------------------------+\n");
    printf("Usable memory before reclaim: %lu MB\n", usable_pre  / 1024 / 1024);
    printf("Usable memory after reclaim:  %lu MB\n", usable_post / 1024 / 1024);
}


// entry function
void kmain(void) {
	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	// Ensure we got a framebuffer.
	if (framebuffer_request.response == NULL
			|| framebuffer_request.response->framebuffer_count < 1
			|| memmap_request.response == NULL
			|| memmap_request.response->entry_count == 0
			|| hhdm_request.response->offset == 0
			|| executable_address_request.response == NULL) {
		hcf();
	}
	

	// Print the pattern to every framebuffer.
	// TODO: handle multiple buffers, only getting one for now
	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
	stdio_init(framebuffer);

	log_debug("executable phys base address: 0x%016lx", executable_address_request.response->physical_base);
	log_debug("executable virt base address: 0x%016lx", executable_address_request.response->virtual_base);
	log_debug("memmap entry count: %lu", memmap_request.response->entry_count);

	init_gdt();
	init_idt();

	memmap_print();
	// asm volatile("int3");
	paging_init();

	bb_instance_init();
	uint64_t address = bb_alloc(sizeof(uint64_t));
	log_debug("hhdm offset: 0x%lx", hhdm_request.response->offset);
	log_debug("got address 0x%016lx from alloc", address);

	// We're done, just hang...
	hcf();
}
