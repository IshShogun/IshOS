#include <stdint.h>

// tss.h
#define IST_NONE 0
#define IST_NMI 1
#define IST_DF  2
#define IST_MC 3

typedef struct tss_entry {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t ;

void init_tss(int index);

