#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm_backends/fb.h>
#include <flanterm.h>
#include <descriptor_tables/gdt.h>
#include <descriptor_tables/idt.h>


// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

// entry function
void kmain(void) {
	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	// Ensure we got a framebuffer.
	if (framebuffer_request.response == NULL
			|| framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}

	// Print the pattern to every framebuffer.
	// TODO: handle multiple buffers, only getting one for now
	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
	stdio_init(framebuffer);

	printf("printf working\n");
	init_gdt();

	init_idt();
	asm volatile("int3");

	// We're done, just hang...
	hcf();
}
