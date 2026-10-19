#include <stdint.h>
#include <utils/limine.h>

//need ot find this.
#define MAX_BB_ORDER 10 //throwaway number

typedef struct list_head {
	struct list_head *prev;
	struct list_head *next;
} list_head_t;

typedef struct bb_page {
	uint64_t order;
	list_head_t siblings;
} bb_page_t;

typedef struct bb_area {
	list_head_t free_list;
	int n_free;
} bb_area_t;

typedef struct bb_instance {
	bb_area_t free_area[MAX_BB_ORDER];	
} bb_instance_t;


extern uint64_t bump_entry;
extern uint64_t bump_page_offset;

bb_instance_t buddy_instance;


//can do something like that
//but isnt N_PAGES variables?? - can calc it it during preprocessing and page size - but depends on machine so i think i need to allocate
page_t pages[N_PAGES];

//i can bump allocate pages and pack metadata in there. seems to make the most sense
//need to write to useable memory and free_list will point to the first page of that buddy_block

void bb_instance_init(){
			
}

void bb_alloc(){}
