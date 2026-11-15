#include <mem/alloc/buddy.h>
#include <mem/alloc/bump.h>
#include <mem/alloc/slab.h>
#include <mem/pfndb.h>


void mm_init(struct limine_memmap_response *memmap){
	pfndb_init(memmap);
	bb_instance_init(memmap);
	slab_init();
}
