#include <limine.h>

extern volatile uint64_t limine_base_revision[];

extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_executable_address_request executable_address_request;
extern volatile struct limine_executable_file_request executable_file_request;
extern volatile struct limine_module_request module_request;

// uint64_t hhdm_offset;
