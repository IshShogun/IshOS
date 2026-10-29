#include <stdio.h>
#include <stdbool.h>

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

void panic(char* msg){
	printf("PANIC: %s\n", msg);
	hcf();
}

void assert(bool assertation, char* err_msg){
	if(!assertation)
		panic(err_msg);
}
