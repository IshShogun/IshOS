#include <stdint.h>

static inline uint64_t min(uint64_t n1, uint64_t n2){
	return n1 > n2 ? n2 : n1;
}

static inline uint64_t log2(uint64_t num){
	uint64_t exponent = 0;
	while(num >> exponent > 1){
		exponent++;
	}

	return exponent;
}
