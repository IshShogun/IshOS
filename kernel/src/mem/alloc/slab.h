#pragma once
#include <utils/lists.h>

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

void slab_init();

