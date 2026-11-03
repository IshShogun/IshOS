#include <stdint.h>
#include <utils/lists.h>

#define ROOT_FLAG 0x01
#define FREE_FLAG 0x02


#define MAX_BB_ORDER 4

typedef struct bb_page {
	uint8_t flags;
	uint64_t order;
	list_head_t siblings;
} bb_page_t;

typedef struct bb_area {
	list_head_t free_list;
	int n_free;
} bb_area_t;

typedef struct bb_instance {
	bb_area_t free_area[MAX_BB_ORDER + 1];	
	//or maybe bb_page_t pointer - we'll see but defo need this. 
	bb_page_t *base_page;
	uint64_t total_pages;
} bb_instance_t;


 
void bb_instance_init();
uint64_t bb_alloc(uint64_t size);
void bb_free(uint64_t address);

