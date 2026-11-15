#pragma once
#include <stdint.h>
#include <utils/lists.h>
#include <mem/paging.h>
#include <mem/alloc/slab.h>


#define MAX_BB_ORDER 4

typedef struct bb_area {
	list_head_t free_list;
	int n_free;
} bb_area_t;

typedef struct bb_instance {
	bb_area_t free_area[MAX_BB_ORDER + 1];	
} bb_instance_t;

 
void bb_instance_init();
uint64_t bb_alloc(uint64_t size);
void bb_free(uint64_t address);


