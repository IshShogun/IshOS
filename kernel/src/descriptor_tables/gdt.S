global gdt_flush

section .text

gdt_flush:
	lgdt (%rax)
	
