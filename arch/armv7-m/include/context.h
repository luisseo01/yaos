#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define NR_CONTEXT_SOFT			8
#define NR_CONTEXT_HARD			8
#define NR_CONTEXT			(NR_CONTEXT_HARD + NR_CONTEXT_SOFT)
#define CONTEXT_SIZE			(NR_CONTEXT * WORD_SIZE)
/* depending on SCB_CCR[STKALIGN] */

#define INIT_IRQFLAG(flag)		((flag) = 0)

#define EXC_RETURN_MSPH	0xfffffff1	/* return to HANDLER mode using MSP */
#define EXC_RETURN_MSPT	0xfffffff9	/* return to THREAD  mode using MSP */
#define EXC_RETURN_PSPT	0xfffffffd	/* return to THREAD  mode using PSP */

#define DEFAULT_PSR			0x01000000

#define INDEX_R0			8

/*  __________
 * | psr      |  |
 * | pc       |  | stack
 * | lr       |  |
 * | r12      |  v
 * | r3       |
 * | r2       |
 * | r1       |
 * | r0       |
 *  ----------
 * | r4 - r11 |
 *  ----------
 */
struct regs {
	unsigned int r4; /* low address */
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r12;
	unsigned int lr;
	unsigned int pc;
	unsigned int psr;
	//unsigned int sp;
};

#define __context_save(task)	do {					\
	__asm__ __volatile__(						\
			"mrs	r12, psp		\n\t"		\
			"stmdb	r12!, {r4-r11}		\n\t"		\
			::: "memory");					\
	__asm__ __volatile__(						\
			"mov	%0, r12			\n\t"		\
			: "=&r"(task->mm.sp)				\
			:: "memory");					\
} while (0)

#define __context_restore(task)	do {					\
	__asm__ __volatile__(						\
			"msr	msp, %0			\n\t"		\
			"mov	r12, #3			\n\t"		\
			"tst	%1, %2			\n\t"		\
			"it	ne			\n\t"		\
			"movne	r12, #2			\n\t"		\
			:: "r"(task->mm.kernel.sp)			\
			, "r"(get_task_flags(task))			\
			, "I"(TASK_PRIVILEGED)				\
			: "memory");					\
	__asm__ __volatile__(						\
			"ldmia	%0!, {r4-r11}		\n\t"		\
			"msr	psp, %0			\n\t"		\
			"ldr	lr, =0xfffffffd		\n\t"		\
			"msr	control, r12		\n\t"		\
			:: "r"(task->mm.sp)				\
			: "r12", "memory");				\
} while (0)

#define __context_prepare()				cli()
#define __context_finish()				sei()

#define __save_curr_context(regs) do {					\
	__asm__ __volatile__(						\
			"push	{lr}			\n\t"		\
			"mov	lr, %0			\n\t"		\
			"stmia	lr!, {r4-r11}		\n\t"		\
			"stmia	lr!, {r0-r3,r12}	\n\t"		\
			"pop	{r4}			\n\t"		\
			"ldr	r5, =1f			\n\t"		\
			"mov	r6, %1			\n\t"		\
			"stmia	lr!, {r4-r6}		\n\t"		\
			"mov	lr, r4			\n\t"		\
			"1:				\n\t"		\
			:: "r"(regs), "I"(DEFAULT_PSR)			\
			: "memory", "lr", "r4", "r5", "r6");		\
} while (0)

#define __set_retval(task, value)					\
	__asm__ __volatile__("str %0, [%1]" :				\
			: "r"(value), "r"(task->mm.sp + INDEX_R0))

#endif /* __CONTEXT_H__ */
