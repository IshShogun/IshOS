#include <limine.h>
#include <utils/limine.h>

#define ELF_PT_LOAD_TYPE 1
#define PTE_ADDRESS_MASK 0x000FFFFFFFFFF000ULL
#define EI_NIDENT 16

#define PAGE_SIZE 4096U

#define ALIGN_UP(x)   (((uint64_t)(x) + PAGE_SIZE - 1) & ~((uint64_t)PAGE_SIZE - 1))
#define ALIGN_DOWN(x) ((uint64_t)(x) & ~((uint64_t)PAGE_SIZE - 1))
#define TO_HHDM(x)    ((uint64_t)(x) + hhdm_request.response->offset)

enum page_flags {
	PAGE_PRESENT = 1U,
	PAGE_RW = 2U,
	PAGE_USER_ACCESS = 4U,	
	PAGE_PWT = 8U,	
	PAGE_PCD = 16U,	
	PAGE_ACCESSED = 32U,	
	PAGE_R = 2048U,	

};

typedef struct elf_header {
        unsigned char   ident[EI_NIDENT];
        uint32_t        type;
        uint32_t        machine;
        uint64_t        version;
        uint64_t        entry;
        uint64_t        phoff;
        uint64_t        shoff;
        uint64_t        flags;
        uint32_t        ehsize;
        uint32_t        phentsize;
        uint32_t        phnum;
        uint32_t        shentsize;
        uint32_t        shnum;
        uint32_t        shstrndx;
} elf_header_t;

typedef struct elf_program_header {
	uint64_t	type;
	uint64_t	flags;
	uint64_t  offset;
	uint64_t  vaddr;
	uint64_t  paddr;
	uint64_t	filesz;
	uint64_t	memsz;
	uint64_t	align;
} elf_program_header_t;

void paging_init();


