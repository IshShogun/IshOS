#include <stdint.h>
#include <utils/lists.h>
#include <mem/alloc/slab.h>
#include <mem/alloc/bump.h>
#include <mem/paging.h>
#include <mem/pfndb.h>

pfndb_t pfndb;

void ballocate_mem_for_buddy_pages(struct limine_memmap_response *memmap){
	uint64_t n_pages_required = pfndb.total_pages * sizeof(page_t) >> 12U;

	for(int i = bump_entry; i < memmap->entry_count; i++){
		struct limine_memmap_entry *entry =  memmap->entries[i];
		if(entry->type != LIMINE_MEMMAP_USABLE){
			continue;
		}	

		uint64_t offset = (i == bump_entry) ? bump_page_offset : 0;

		uint64_t base_pfn = ALIGN_UP(entry->base + offset * PAGE_SIZE) >> 12U;
		uint64_t end_pfn = ALIGN_DOWN(entry->base + entry->length) >> 12U;


		if(n_pages_required > end_pfn - base_pfn)	
			continue;

		
		pfndb.base_page = (page_t*) TO_HHDM(base_pfn * PAGE_SIZE);
		return;
	}
	
	panic("NOT ENOUGH MEMORY FOR BUDDY ALLOCATOR");
}

static inline uint64_t get_max_phys_addr(struct limine_memmap_response *memmap){
	uint64_t max_addr = 0;
	for(int i = 0; i < memmap->entry_count; i++){
		struct limine_memmap_entry *entry = memmap->entries[i];
		switch (entry->type){
			case LIMINE_MEMMAP_USABLE: break;
			case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: break;
			case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES: break;
			case LIMINE_MEMMAP_ACPI_RECLAIMABLE: break;
			default: 
				continue;
		}
		
		uint64_t top = entry->base + entry->length;
		if(top > max_addr)
			max_addr = top;

	}

	return ALIGN_UP(max_addr);
}

void pfndb_init(struct limine_memmap_response *memmap){
	uint64_t max_phys_addr = get_max_phys_addr(memmap);

	pfndb.total_pages = max_phys_addr >> 12U;
	ballocate_mem_for_buddy_pages(memmap);

	//init for buddy -- they all start in buddy
	for(uint64_t i = 0; i < pfndb.total_pages; i++){
		page_t *page = pfndb.base_page + i;
		*page = (struct page){
			.order = 0,
			.flags = 0,
		};
		list_head_init(&page->siblings);
	}
}

