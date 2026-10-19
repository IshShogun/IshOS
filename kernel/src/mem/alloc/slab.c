// #include <stdint.h>
//
//
// #define MAX_CACHE_ORDER 8
// #define ORDER_OF_HALF_PAGE_SIZE 0
//
// typedef struct kmem_cache {
//
//
// } kmem_cache_t;
//
// uint64_t bb_alloc(uint64_t size);
//
// uint64_t kmem_cache_alloc(kmem_cache_t* _){
// 	return 0;
// }
//
//
//
// extern uint64_t HHDM_OFFSET;
// kmem_cache_t* slab_cache_for_size[MAX_CACHE_ORDER];
//
// void* kmalloc(uint64_t size){
// 	unsigned order = 0;
//
// 	while(size != 0){
// 		order++;
// 		size /= 2; 
// 	}
//
// 	//set 0 so always goes here for now,  ge for now so ensure it always goes
// 	if(order >= ORDER_OF_HALF_PAGE_SIZE){
// 		uint64_t paddr = bb_alloc(size);
// 		return (void*) paddr + HHDM_OFFSET;
// 	}
//
// 	kmem_cache_t *cache = slab_cache_for_size[order];
// 	uint64_t paddr = kmem_cache_alloc(cache);
//
// 	return (void*) (paddr + HHDM_OFFSET);
// }
