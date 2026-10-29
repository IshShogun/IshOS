#include <stdint.h>
#include <stdio.h>
#include <utils/limine.h>
#include <mem/paging.h>
#include <mem/alloc/bump.h>
#include <panic.h>
#include <utils/lists.h>
#include <utils/math.h>
#include <mem/alloc/buddy.h>


extern uint64_t bump_entry;
extern uint64_t bump_page_offset;

bb_instance_t buddy_instance;

//most of my difficulties are coming from how i dow this. 
void ballocate_mem_for_buddy_pages(struct limine_memmap_response *memmap){
	//hold on how many pages do i actually need
	uint64_t n_pages_required = buddy_instance.total_pages * sizeof(bb_page_t) >> 12U;
	uint64_t block_order = log2(n_pages_required) + 1;
	uint64_t block_pages = 1 << block_order;

	log_debug("n_pages_required: %lu, log(n_pages_required): %lu, block order: %i, block pages: %lu", n_pages_required, log2(n_pages_required), block_order, block_pages);

	//now i need to find a block of that size and store the pfn? BASE >> 12U.
	for(int i = bump_entry; i < memmap->entry_count; i++){
		struct limine_memmap_entry *entry =  memmap->entries[i];
		if(entry->type != LIMINE_MEMMAP_USABLE){
			continue;
		}	

		uint64_t offset = (i == bump_entry) ? bump_page_offset : 0;

		uint64_t base_pfn = ALIGN_UP(entry->base + offset * PAGE_SIZE) >> 12U;
		uint64_t end_pfn = ALIGN_DOWN(entry->base + entry->length) >> 12U;

		log_debug("available pages: %i", end_pfn - base_pfn);

		if(block_pages > end_pfn - base_pfn)	
			continue;

		//now i need to fit it in here. first i need to get the first 2^(binstance_order) aligned number in block
		uint64_t remainder = base_pfn % block_pages;
		uint64_t nearest_aligned_pfn = base_pfn + block_pages - remainder;

		if(nearest_aligned_pfn + block_pages > end_pfn)
			continue;
		
		log_debug("nearest_aligned_pfn: %i", nearest_aligned_pfn);
		log_debug("hhdm address: 0x%016lx", TO_HHDM(nearest_aligned_pfn * PAGE_SIZE));
		buddy_instance.base_page = (bb_page_t*) TO_HHDM(nearest_aligned_pfn * PAGE_SIZE);
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


uint64_t init_bb_range(uint64_t pfn, uint64_t order, struct limine_memmap_response *memmap){
	//now i set flags and add to freelist in useable ram.
	uint64_t buddy_pfn = FROM_HHDM(buddy_instance.base_page) >> 12U;
	uint64_t bump_pfn = (ALIGN_UP(memmap->entries[bump_entry]->base) >> 12U) + bump_page_offset;

	bb_page_t *page = buddy_instance.base_page + pfn;

	//page table reserved
	if(pfn < bump_pfn){
		page->flags = 0;
		page->order = 0;
		return 1;
	} 

	//buddy page array reserved
	if(buddy_pfn >= pfn && buddy_pfn <= pfn + (1 << order)){
		for(int i = pfn; i < buddy_pfn ; i++){
			bb_page_t *page = buddy_instance.base_page + i;
			page->order = 0;
			page->flags = ROOT_FLAG | FREE_FLAG;
		}

		uint64_t buddy_order = log2(buddy_instance.total_pages * sizeof(bb_page_t) >> 12U) + 1;
		for(int i = 0; i < (1 << buddy_order); i++){
			bb_page_t *page = buddy_instance.base_page + buddy_pfn + i;
			page->order = 0;
			page->flags = 0;
		}
		
		return (buddy_pfn - pfn) + (1 << buddy_order);
	}



	page->order = order;
	page->flags = ROOT_FLAG | FREE_FLAG;
	list_head_insert_after(&page->siblings, &buddy_instance.free_area[order].free_list);
	
	return (1 << order);
}



void bb_instance_init(){
	struct limine_memmap_response *memmap = memmap_request.response;
	uint64_t max_phys_addr = get_max_phys_addr(memmap);
	log_debug("MAX PHYS ADDRESS: 0x%016lx", max_phys_addr);

	buddy_instance.total_pages = max_phys_addr >> 12U;
	log_debug("total pages avail: 0x%016lx", buddy_instance.total_pages);
	ballocate_mem_for_buddy_pages(memmap);


	for(int i = 0; i < MAX_BB_ORDER; i++){
		bb_area_t *area = &buddy_instance.free_area[i];
		list_head_init(&area->free_list);
	}

	printf("I AM HERE\n");
	//init everything as nonroot so not useable memory/pages bounce on free
	//explicity set root flags.
	printf("base page: 0x%016lx\n", buddy_instance.base_page);
	for(uint64_t i = 0; i < buddy_instance.total_pages; i++){
		bb_page_t *page = buddy_instance.base_page + i;
		*page = (struct bb_page){
			.order = 0,
			.flags = 0,
		};
		list_head_init(&page->siblings);
	}	
	printf("I AM HERE\n");

	for(uint64_t i = 0; i < memmap->entry_count; i++){
		struct limine_memmap_entry *entry =  memmap->entries[i];
		if(entry->type != LIMINE_MEMMAP_USABLE)
			continue;

		uint64_t base_pfn = ALIGN_UP(entry->base) >> 12U;
		uint64_t end_pfn = ALIGN_DOWN(entry->base + entry->length) >> 12U;

		uint64_t pfn = base_pfn;
		while(pfn < end_pfn){
			uint64_t max_fit = log2(end_pfn - pfn); //biggest power of 2 block that can fit in range
			uint64_t alignment = log2(pfn); //biggest power of 2 flipped
			uint64_t order = min(MAX_BB_ORDER, min(max_fit, alignment)); //bigger power of 2 is lower power of 2 aligned

			uint64_t frames_initaliseed = init_bb_range(pfn, order, memmap);

			pfn += frames_initaliseed;
		}
	}

	log_debug("buddy initalised");
}

void bb_alloc(){}
