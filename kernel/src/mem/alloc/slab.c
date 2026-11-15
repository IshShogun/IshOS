#include <stdint.h>
#include <utils/lists.h>
#include <mem/paging.h>

#define MAX_CACHE_ORDER 8
#define ORDER_OF_HALF_PAGE_SIZE 0

typedef struct kmem_cache {
	list_head_t slabs_partial;
	list_head_t slabs_full;
	list_head_t slabs_free;
	uint64_t objsize;
} kmem_cache_t;

typedef struct slab {
	list_head_t siblings;
	void* s_mem;	
	uint64_t in_use;
	//i can use a freelist because this is a size cache not a type cache
	list_head_t freelist;	
} slab_t;


uint64_t bb_alloc(uint64_t size);

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
