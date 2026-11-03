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
//since im just reserving the memory. i prob don't need to do it this way at all. 
//just find the base memory, and reserve the whole chunk. start from the end.
void ballocate_mem_for_buddy_pages(struct limine_memmap_response *memmap){
	//hold on how many pages do i actually need
	uint64_t n_pages_required = buddy_instance.total_pages * sizeof(bb_page_t) >> 12U;

	//now i need to find a block of that size and store the pfn? BASE >> 12U.
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

		
		buddy_instance.base_page = (bb_page_t*) TO_HHDM(base_pfn * PAGE_SIZE);
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
			list_head_insert_after(&page->siblings, &buddy_instance.free_area[0].free_list);
		}

		uint64_t n_pages_for_buddy = buddy_instance.total_pages * sizeof(bb_page_t) >> 12U;
		for(int i = 0; i < n_pages_for_buddy; i++){
			bb_page_t *page = buddy_instance.base_page + buddy_pfn + i;
			page->order = 0;
			page->flags = 0;
		}
		
		return (buddy_pfn - pfn) + n_pages_for_buddy;
	}



	page->order = order;
	page->flags = ROOT_FLAG | FREE_FLAG;
	list_head_insert_after(&page->siblings, &buddy_instance.free_area[order].free_list);
	
	return (1 << order);
}

uint64_t get_alignment(uint64_t number){
	//ensures at least one flipped
	if(number == 0)
		return 64;
	//find lowest bit
	uint64_t order = 0;
	uint64_t mask = 0x01;

	while((mask & number) == 0){
		mask = mask << 1;
		order++;
	}

	return order;
}

void bb_instance_init(){
	struct limine_memmap_response *memmap = memmap_request.response;
	uint64_t max_phys_addr = get_max_phys_addr(memmap);

	buddy_instance.total_pages = max_phys_addr >> 12U;
	ballocate_mem_for_buddy_pages(memmap);


	for(int i = 0; i <= MAX_BB_ORDER; i++){
		bb_area_t *area = &buddy_instance.free_area[i];
		list_head_init(&area->free_list);
	}

	//init everything as nonroot so not useable memory/pages bounce on free
	//explicity set root flags.
	for(uint64_t i = 0; i < buddy_instance.total_pages; i++){
		bb_page_t *page = buddy_instance.base_page + i;
		*page = (struct bb_page){
			.order = 0,
			.flags = 0,
		};
		list_head_init(&page->siblings);
	}	

	for(uint64_t i = 0; i < memmap->entry_count; i++){
		struct limine_memmap_entry *entry =  memmap->entries[i];
		if(entry->type != LIMINE_MEMMAP_USABLE)
			continue;

		uint64_t base_pfn = ALIGN_UP(entry->base) >> 12U;
		uint64_t end_pfn = ALIGN_DOWN(entry->base + entry->length) >> 12U;

		uint64_t pfn = base_pfn;
		while(pfn < end_pfn){
			uint64_t max_fit = log2(end_pfn - pfn); 
			uint64_t alignment = get_alignment(pfn); 			
			uint64_t order = min(MAX_BB_ORDER, min(max_fit, alignment)); 
			uint64_t frames_initaliseed = init_bb_range(pfn, order, memmap);

			pfn += frames_initaliseed;
		}
	}

	log_debug("buddy initalised");
}


void map_to_free_pages(uint64_t start, uint64_t end){
	bb_area_t *area = &buddy_instance.free_area[0];
	uint64_t addr = start;
	while(addr < end){
		uint64_t pfn = addr >> 12U;
		bb_page_t *page = buddy_instance.base_page + pfn;
		page->order = 0;
		page->flags = ROOT_FLAG | FREE_FLAG;
		list_head_insert_after(&page->siblings, &area->free_list);
		
		addr += PAGE_SIZE;
	}
}


bb_page_t *get_page_struct_from_list_head(list_head_t *entry)
{
    return list_entry(entry, bb_page_t, siblings);
}


//best as a macro i think
static inline uint64_t get_phys_addr_from_page_struct(bb_page_t *page){
	return ((uint64_t) page - (uint64_t) buddy_instance.base_page) * PAGE_SIZE / sizeof(bb_page_t);
}

uint64_t bb_block_decompose(uint64_t current_order, uint64_t target_order, bb_area_t *area){
	if(area->n_free == 0)
		return 0;

	//find nearest aligned block with target_order in block
	uint64_t counter = 0;
	bb_page_t *page = get_page_struct_from_list_head(area->free_list.next);
	while(counter < area->n_free){
		uint64_t base = get_phys_addr_from_page_struct(page);
		uint64_t end = base + (1 << current_order) * PAGE_SIZE;

		uint64_t remainder = base % (1 << target_order) * PAGE_SIZE;
		uint64_t aligned = base + (1 << target_order) * PAGE_SIZE - remainder;

		if(aligned + (1 << target_order) * PAGE_SIZE > end){
			page = get_page_struct_from_list_head(page->siblings.next);	
			counter++;
			continue;
		}

		page = (bb_page_t*) TO_HHDM(aligned);
		page->order = target_order;
		page->flags = ROOT_FLAG | FREE_FLAG;
		list_head_insert_after(&page->siblings, &buddy_instance.free_area[target_order].free_list);
		map_to_free_pages(base, aligned);
		map_to_free_pages(aligned + (1 << target_order) * PAGE_SIZE, end);
		
		return aligned;
	}

	return 0;
}


uint64_t bb_block_compose(uint64_t current_order, uint64_t target_order, bb_area_t *area){
	uint64_t required_blocks = 1 << (target_order - current_order);
	if(required_blocks > area->n_free)
		return 0;

	//set base and end -> needed for block. base is target aligned
	list_head_t *entry = &area->free_list;
	uint64_t end, aligned, n_blocks = 0;
	uint64_t base = get_phys_addr_from_page_struct(get_page_struct_from_list_head(entry->next));

	//find contingous range of blocks that fit target order
	while(list_head_empty(entry->next) && n_blocks < required_blocks){
		entry = entry->next;
		bb_page_t *page = get_page_struct_from_list_head(entry);
		if(n_blocks == 0){
			uint64_t remainder = base % (1 << target_order);
			aligned = base + (1 << target_order) - remainder;
		} 

		uint64_t next = aligned * n_blocks * (1 << current_order) * PAGE_SIZE;
		end = get_phys_addr_from_page_struct(page) + (1 << current_order) * PAGE_SIZE;

		if(next >= end){
			base = get_phys_addr_from_page_struct(get_page_struct_from_list_head(entry->next));
			n_blocks = 0;
			continue;
		}

		n_blocks++;
	}

	if(n_blocks < required_blocks){
		log_debug("not able to compose blocks for order: %i, from order: %i", target_order, current_order);
		return 0;
	}


	bb_page_t *page = buddy_instance.base_page + (base >> 12U);
	//TODO: defo need a list_head empty check here - a whole loop most likely
	for(int i = 0; i < required_blocks; i++){
		list_head_t *entry = &page->siblings;
		list_head_remove(entry);
		page = get_page_struct_from_list_head(entry->next);
		page->flags = 0;
	}

	int pfn = aligned >> 12U;
	bb_page_t *aligned_page = buddy_instance.base_page + pfn;
	aligned_page->order = target_order;
	aligned_page->flags = ~FREE_FLAG | ROOT_FLAG;
	list_head_insert_after(&aligned_page->siblings, &buddy_instance.free_area[target_order].free_list);
	map_to_free_pages(base, aligned);
	map_to_free_pages(aligned + (1 << target_order) * PAGE_SIZE, end);
	return aligned;
}

uint64_t bb_alloc(uint64_t size){
	//this is the log in terms of page frames
	uint64_t order = log2(size >> 12U);
	log_debug("order: %i", order);
	if(order > MAX_BB_ORDER)
		order = MAX_BB_ORDER;

	log_debug("order: %i", order);
	bb_area_t *area = &buddy_instance.free_area[order];
	//more readable and checks for list_head null bugs too.
	uint64_t alloc_address = 0;
	if(list_head_empty(&area->free_list)){
		for(int i = MAX_BB_ORDER; i >= 0; i--){
			bb_area_t *area = &buddy_instance.free_area[i];
			if(list_head_empty(&area->free_list))
				continue;
			
			if(i > order){
				alloc_address = bb_block_compose(i, order, area);
			}	else {
				alloc_address = bb_block_decompose(i, order, area);
			}

			log_debug("alloc_address: 0x%016lx", alloc_address);
			if(alloc_address != 0)
				return TO_HHDM(alloc_address);

		}

		panic("not enough memory");
	}

	list_head_remove(area->free_list.next);
	alloc_address = get_phys_addr_from_page_struct(get_page_struct_from_list_head(area->free_list.next));
	log_debug("alloc_address: 0x%016lx", alloc_address);
	return TO_HHDM(alloc_address);
}

void bb_free(uint64_t address){
	bb_page_t *page = buddy_instance.base_page + (FROM_HHDM(address) >> 12U);	

	//cant free non root page or already free page
	//so we bounce when page is non_root or already free
	if((page->flags & FREE_FLAG) == 1  || (page->flags & ROOT_FLAG) == 0){
		panic("freeing non-freeable address - either non root page or already free");
	}

	page->flags = FREE_FLAG | ROOT_FLAG;
	list_head_insert_after(&page->siblings, &buddy_instance.free_area[page->order].free_list);
}
