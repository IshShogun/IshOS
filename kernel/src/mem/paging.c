#include <limine.h>
#include <panic.h>
#include <stdio.h>
#include <utils/limine.h>
#include <mem/alloc/bump.h>
#include <mem/paging.h>

//x86 only for now cause i have no idea what paging looks like under other architectures. so no meaningfull way to abstract.
//not supporting large pages for now

uint64_t get_next_page_table_and_allocate(uint64_t pt_vaddr, uint64_t idx){
	uint64_t *pte = (uint64_t*) pt_vaddr + idx;
	uint64_t next_pt_paddr = *pte & PTE_ADDRESS_MASK;
	if(next_pt_paddr == 0){
		next_pt_paddr = bump_alloc_page();
		*pte = next_pt_paddr | PAGE_PRESENT | PAGE_RW;
	}


	return TO_HHDM(next_pt_paddr);
}

void map_pages(uint64_t pml4_vaddr, uint64_t vaddr, uint64_t paddr, uint64_t flags){
			//TODO: this'lll depend on cpuid, and M -> max addressable bit.
			uint64_t pml4_idx = (vaddr >> 39) & 0x01ff; //bits 47 to 39
			uint64_t pdpt_idx = (vaddr >> 30) & 0x01ff; //bits 38 to 30
			uint64_t pd_idx = (vaddr >> 21) & 0x01ff; //bits 29 to 21
			uint64_t pt_idx = (vaddr >> 12) & 0x01ff; //bits 20 to 12
			uint64_t offset = vaddr & 0x0fff; //bits 11 - 0;
			
			if(offset != 0){
				//panic
				panic("PAGING ERROR: offset not 0\n");
			}

			uint64_t pdpt_vaddr = get_next_page_table_and_allocate(pml4_vaddr, pml4_idx);
			uint64_t pd_vaddr = get_next_page_table_and_allocate(pdpt_vaddr, pdpt_idx);
			uint64_t pt_vaddr = get_next_page_table_and_allocate(pd_vaddr, pd_idx);

			uint64_t *pte = (uint64_t*) pt_vaddr + pt_idx;

			//first 12 bits are already 0, as its page aligned
			*pte = paddr | flags;
}


void paging_init(){
	uint64_t pml4_vaddr = TO_HHDM(bump_alloc_page());

	struct limine_memmap_response *memmap_response = memmap_request.response;

	//map hhdm
	for(int i = 0; i < memmap_response->entry_count; i++){
		struct limine_memmap_entry *entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_RESERVED)   continue;
		if (entry->type == LIMINE_MEMMAP_BAD_MEMORY) continue;

		uint64_t pbase_page = ALIGN_UP(entry->base);
		uint64_t pend_page = ALIGN_DOWN(entry->base + entry->length);

		uint64_t addr = pbase_page;
		while(addr < pend_page){
			uint64_t vaddr = TO_HHDM(addr);	

			map_pages(pml4_vaddr, vaddr, addr, PAGE_PRESENT | PAGE_RW);

			addr += PAGE_SIZE;
		}
	}

	//map elf
	struct limine_executable_file_response *kernel_exec = executable_file_request.response;
	struct limine_executable_address_response *kernel_addr = executable_address_request.response;

	elf_header_t *kernel_elf_header = (elf_header_t*)  kernel_exec->executable_file->address;
	uint64_t ph_start = (uint64_t) kernel_elf_header + kernel_elf_header->phoff;	

	for(uint32_t i = 0; i < kernel_elf_header->phnum; i++){
		elf_program_header_t *ph = (elf_program_header_t*) ph_start + i;
		if(ph->type != ELF_PT_LOAD_TYPE)
			continue;
		
		uint64_t seg_vaddr = ph->vaddr;
		uint64_t seg_paddr = kernel_addr->physical_base + (ph->vaddr - kernel_addr->virtual_base);
	
		uint64_t start_page = ALIGN_DOWN(seg_paddr);
		uint64_t end_page = ALIGN_UP(seg_paddr + ph->memsz);
		
		uint64_t n_pages = (end_page - start_page) >> 12U;
		uint64_t paddr = start_page;

		for(uint64_t i = 0; i < n_pages; i++){
			uint64_t vaddr = ALIGN_DOWN(seg_vaddr + i * PAGE_SIZE);
			map_pages(pml4_vaddr, vaddr, paddr, PAGE_PRESENT | PAGE_RW);

			paddr += PAGE_SIZE;
		}

	}

	log_debug("paging initalised\n");
}
