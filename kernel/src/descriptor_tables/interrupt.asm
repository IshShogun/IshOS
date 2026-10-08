extern  handle_isr

%macro ISR 1
global ISR_%1

ISR_%1:
	; should be pushed here no.... -- it is
	push 0 ; dummy error
	push %1 ; pushing INT number for debug
	jmp isr_dispatch

%endmacro

%macro ISR_ERR 1
global ISR_ERR_%1

ISR_ERR_%1:
	; error already pushed
	push %1 
	jmp isr_dispatch

%endmacro


isr_dispatch:
	; SS, RSP, RFLAGS, CS, RIP, ERROR CODE, INTERRUPT NUMBER where pushed before call to isr%1 , jmp just changes running rip 
	push rax
	push rbx
	push rcx 
	push rdx 
	push rdi
	push rsi 
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	
	mov rdi, rsp
	call handle_isr

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax

	add rsp, 16 ; error code and interrupt number still on the stack, move that shit up

	iretq

	
 ;0-7 no error, 8 error, 10-14 error, 17 error, 21 error


 ISR 0 ; 
 ISR 1 ;
 ISR 2 ; 
 ISR 3 ; 
 ISR 4 ; 
 ISR 5 ; 
 ISR 6 ; 
 ISR 7 ; 
 ISR_ERR 8 ; 
 ISR 9 ; 
 ISR_ERR 10 ; 
 ISR_ERR 11; 
 ISR_ERR 12; 
 ISR_ERR 13; 
 ISR_ERR 14; 
 ISR 15 ; 
 ISR 16 ; 
 ISR_ERR 17; 
 ISR 18 ; 
 ISR 19 ; 
 ISR 20 ; 
 ISR_ERR 21 ; 
