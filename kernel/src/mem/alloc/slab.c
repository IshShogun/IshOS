#include <stdint.h>
#include <utils/lists.h>
#include <mem/paging.h>
#include <mem/pfndb.h>
#include <mem/alloc/buddy.h>
#include <mem/alloc/slab.h>

extern uint64_t HHDM_OFFSET;
kmem_cache_t* size_caches[MAX_CACHE_ORDER];


void slab_init(){
	for(int i = 0; i < MAX_CACHE_ORDER; i++){
		kmem_cache_t* cache = size_caches[i];
		list_head_init(&cache->slabs_full);
		list_head_init(&cache->slabs_partial);
		list_head_init(&cache->slabs_free);
		cache->objsize = 1 << i;

		//add pages to slabs_free - but how many?
		for(int j = 0; j < 10; j++){
			slab_t *slab = (slab_t*)bb_alloc(PAGE_SIZE);

			page_t *page = pfndb.base_page + (FROM_HHDM(slab) >> 12U);
			page->flags = page->flags | SLAB_FLAG;
			page->slab_cache = cache;

			list_head_init(&slab->siblings);
			list_head_init(&slab->freelist);
			slab->s_mem = (void*) ((char*)slab + sizeof(slab_t));
			slab->in_use = 0;

			uint64_t addr = (uint64_t)slab->s_mem;
			while(addr <= (uint64_t)slab->s_mem + PAGE_SIZE){
				list_head_init((list_head_t*)addr);	
				list_head_insert_after((list_head_t*)addr, &slab->freelist);
				addr += cache->objsize;
			}
			list_head_insert_after(&slab->siblings, &cache->slabs_free);
		}
		
	}
}


uint64_t kmem_cache_alloc(kmem_cache_t* cache){
	if(list_head_empty(&cache->slabs_partial)){
		if(list_head_empty(&cache->slabs_free)){
			slab_t *slab = (slab_t*)bb_alloc(PAGE_SIZE);

			page_t *page = pfndb.base_page + (FROM_HHDM(slab) >> 12U);
			page->flags = page->flags | SLAB_FLAG;
			page->slab_cache = cache;

			list_head_init(&slab->siblings);
			list_head_init(&slab->freelist);
			slab->s_mem = (void*) ((char*)slab + sizeof(slab_t));
			slab->in_use = 0;

			uint64_t addr = (uint64_t)slab->s_mem;
			while(addr <= (uint64_t)slab->s_mem + PAGE_SIZE){
				list_head_init((list_head_t*)addr);	
				list_head_insert_after((list_head_t*)addr, &slab->freelist);
				addr += cache->objsize;
			}
			list_head_insert_after(&slab->siblings, &cache->slabs_free);
		} 

		//now here next should be the beginning of object i think as sibilings is at top?
		slab_t *slab = (slab_t*) cache->slabs_free.next;
		list_head_remove(&slab->siblings);
		list_head_insert_after(&slab->siblings, &cache->slabs_partial);
		uint64_t addr = (uint64_t) slab->freelist.next;
		list_head_remove(slab->freelist.next);
		slab->in_use++;
		if(list_head_empty(&slab->freelist)){
			list_head_remove(&slab->siblings);
			list_head_insert_after(&slab->siblings, &cache->slabs_full);
		}
		return addr;
	}
	
	slab_t *slab = (slab_t*) cache->slabs_partial.next;
	uint64_t addr = (uint64_t) slab->freelist.next;
	list_head_remove(slab->freelist.next);
	slab->in_use++;
	if(list_head_empty(&slab->freelist)){
		list_head_remove(&slab->siblings);
		list_head_insert_after(&slab->siblings, &cache->slabs_full);
	}

	return addr;
}

//is vaddr part of a slab or part of the buddy alloc	
void kfree(uint64_t vaddr){
	page_t *page = pfndb.base_page + (FROM_HHDM(vaddr) >> 12U);
	if(page->flags & SLAB_FLAG){
		return;
	}

	bb_free(vaddr);
}

void* kmalloc(uint64_t size){
	unsigned order = 0;

	while(size != 0){
		order++;
		size /= 2; 
	}

	//set 0 so always goes here for now,  ge for now so ensure it always goes
	if(order >= ORDER_OF_HALF_PAGE_SIZE){
		uint64_t paddr = bb_alloc(size);
		return (void*) paddr + HHDM_OFFSET;
	}

	kmem_cache_t *cache = size_caches[order];
	uint64_t paddr = kmem_cache_alloc(cache);

	return (void*) (paddr + HHDM_OFFSET);
}
