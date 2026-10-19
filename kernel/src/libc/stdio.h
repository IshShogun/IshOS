#include <limine.h>

int printf(const char *fmt, ...);
void stdio_init(struct limine_framebuffer *framebuffer);

#define log_debug(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
