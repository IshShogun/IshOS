#pragma once
#include <stdint.h> 
#include <utils/lists.h>
#include <mem/alloc/slab.h>
#include <mem/paging.h>

#define ROOT_FLAG 0x01
#define FREE_FLAG 0x02
#define SLAB_FLAG 0x04


typedef struct page {
	uint8_t flags;
	uint64_t order;
	union {
		list_head_t siblings;
		kmem_cache_t *slab_cache;
	};
} page_t;


typedef struct pfndb {
	uint64_t total_pages;
	page_t *base_page;
} pfndb_t;

extern pfndb_t pfndb;
void pfndb_init(struct limine_memmap_response *memmap);

static inline page_t *get_page_struct_from_list_head(list_head_t *entry)
{
    return list_entry(entry, page_t, siblings);
}

//best as a macro i think
static inline uint64_t get_phys_addr_from_page_struct(page_t *page){
	return ((uint64_t) page - (uint64_t) pfndb.base_page) * PAGE_SIZE / sizeof(page_t);
}

