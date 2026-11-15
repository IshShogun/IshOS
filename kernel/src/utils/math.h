#pragma once
#include <stdint.h>

static inline uint64_t min(uint64_t n1, uint64_t n2){
	return n1 > n2 ? n2 : n1;
}

static inline uint64_t get_alignment(uint64_t number){
	//ensures at least one flipped
	if(number == 0)
		return 64;
	//find lowest bit
	uint64_t order = 0;
	uint64_t mask = 0x01;

	while((mask & number) == 0){
		mask = mask << 1;
		order++;
	}

	return order;
}

static inline uint64_t log2(uint64_t num){
	uint64_t exponent = 0;
	while(num >> exponent > 1){
		exponent++;
	}

	return exponent;
}
