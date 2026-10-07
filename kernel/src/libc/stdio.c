#include "limine.h"
#include <flanterm_backends/fb.h>
#include <stdarg.h>

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 1
#define NANOPRINTF_USE_FLOAT_SINGLE_PRECISION 0
//super interesting. the nanoprintf library will only supply the implementations if this is only
//preprocessor checks defines and passes implementations to compiler only if this is here - text subsition/macro
#define NANOPRINTF_IMPLEMENTATION
#include <nanoprintf.h>

struct flanterm_context *ft_ctx;

static const char cr = '\r';

//vpprintf passes a pointer to the ctx, but we dont use it as we have limine framebuffer 
//thats what the anonymus void pointers for
static void io_putc(int c, void *_){
	if (ft_ctx != NULL){
		//char_address is in rdi - x86_64
		if((char)c == '\n')
			flanterm_write(ft_ctx, &cr, 1);


		flanterm_write(ft_ctx, (char*)&c, 1);
	}	

	return;
}

//... declares it as a varadic argument
int printf(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);

	int ret = npf_vpprintf(io_putc, NULL, fmt, args);
	va_end(args);	

	return ret;
}

void stdio_init(struct limine_framebuffer *framebuffer){

	ft_ctx = flanterm_fb_init(
			NULL,
			NULL,
			(uint32_t*) framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,
			framebuffer->red_mask_size, framebuffer->red_mask_shift,
			framebuffer->green_mask_size, framebuffer->green_mask_shift,
			framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
			NULL,
			NULL, NULL,
			NULL, NULL,
			NULL, NULL,
			NULL, 0, 0, 1,
			0, 0,
			0,
			0,
			true
			);
}
