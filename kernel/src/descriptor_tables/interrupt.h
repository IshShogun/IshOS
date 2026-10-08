#include <stdint.h>

typedef struct interrupt_frame {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rsi, rdi, rdx, rcx, rbx, rax;	
	uint64_t interrupt_id, error_code, rip, cs, rflags, rsp, ss;
} __attribute__((packed)) interrupt_frame_t;                                                                                                             

typedef void (*isr_t)(interrupt_frame_t*);

extern void ISR_0(void);
extern void ISR_1(void);
extern void ISR_2(void);
extern void ISR_3(void);
extern void ISR_4(void);
extern void ISR_5(void);
extern void ISR_6(void);
extern void ISR_7(void);
extern void ISR_ERR_8(void);
extern void ISR_9(void);
extern void ISR_ERR_10(void);
extern void ISR_ERR_11(void);
extern void ISR_ERR_12(void);
extern void ISR_ERR_13(void);
extern void ISR_ERR_14(void);
extern void ISR_15(void);
extern void ISR_16(void);
extern void ISR_ERR_17(void);
extern void ISR_18(void);
extern void ISR_19(void);
extern void ISR_20(void);
extern void ISR_ERR_21(void);

// CPU exception vectors
#define INT_DE   0    // Divide Error
#define INT_DB   1    // Debug
#define INT_NMI  2    // Non-Maskable Interrupt
#define INT_BP   3    // Breakpoint
#define INT_OF   4    // Overflow
#define INT_BR   5    // BOUND Range Exceeded
#define INT_UD   6    // Invalid Opcode
#define INT_NM   7    // Device Not Available
#define INT_DF   8    // Double Fault                  (error code)
#define INT_CSO  9    // Coprocessor Segment Overrun   (legacy, never fires)
#define INT_TS   10   // Invalid TSS                   (error code)
#define INT_NP   11   // Segment Not Present           (error code)
#define INT_SS   12   // Stack-Segment Fault           (error code)
#define INT_GP   13   // General Protection            (error code)
#define INT_PF   14   // Page Fault                    (error code)
                      // 15 reserved
#define INT_MF   16   // x87 Floating-Point
#define INT_AC   17   // Alignment Check               (error code)
#define INT_MC   18   // Machine Check
#define INT_XM   19   // SIMD Floating-Point
#define INT_VE   20   // Virtualization
#define INT_CP   21   // Control Protection            (error code)
