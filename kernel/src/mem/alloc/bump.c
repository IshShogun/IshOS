#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limine.h>
#include <utils/limine.h>
#include <panic.h>
#include <mem/paging.h>

uint64_t bump_entry = 0;
uint64_t bump_page_offset = 0;

//gotta be careful not to realloc the page here
uint64_t bump_alloc_page(){
	struct limine_memmap_response *memmap_response = memmap_request.response;

	while(bump_entry < memmap_response->entry_count){
		struct limine_memmap_entry *entry = memmap_response->entries[bump_entry];
		if(entry->type != LIMINE_MEMMAP_USABLE){
			bump_entry++;
			bump_page_offset = 0;
			continue;
		}

		uint64_t start_page = ALIGN_UP(entry->base);
		uint64_t end_page = ALIGN_DOWN(entry->base + entry->length);

		//prob bit shift by log2 PAGE_SIZE mmore efficent here than / PAGE_SIZE
		uint64_t n_pages = (end_page - start_page) >> 12U;

		if(n_pages == 0){
			bump_entry++;
			continue;
		}

		uint64_t paddr = start_page + bump_page_offset * PAGE_SIZE;

		memset((void*)TO_HHDM(paddr), 0, PAGE_SIZE);

		//return vaddr TO_HHDM
		if(bump_page_offset != n_pages - 1){
			bump_page_offset++;
			return paddr;
		}
	
		bump_entry++;
		bump_page_offset = 0;
	
		return paddr;
	}

	panic("not enough useable ram to boot");
	return 0;
}
