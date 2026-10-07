%macro ISR %1

isr%1:
	push 0 ; dummy error
	push %1 ; pushing int number for debug
	jmp isr_dispatch

%endmacro

%macro ISR_ERR %1

isr_err%1:
	; error already pushed
	push %1 
	jmp isr_dispatch


isr_dispatch:
	; push registers
	; call handler
	; pop and iretq i assume
 
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
 ISR 21 ; 
