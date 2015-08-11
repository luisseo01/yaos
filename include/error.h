#ifndef __ERROR_H__
#define __ERROR_H__

/* return error code */
#define ERR_RETRY		1
#define ERR_UNDEF		2
#define ERR_ALLOC		3
#define ERR_RANGE		4
#define ERR_PARAM		5
#define ERR_PERM		6
#define ERR_DUP			7
#define ERR_PATH		8
#define ERR_ATTR		9
#define ERR_CREATE		10
#define ERR_UNKNOWN		11

#define freeze()		while (1)
#define panic()			while (1)

#ifdef CONFIG_DEBUG
#define debug(fmt) do { \
	extern void __putc_debug(int c); \
	void (*temp)(int) = putchar; \
	putchar = __putc_debug; \
	printf("%s:%s():%d: ", __FILE__, __func__, __LINE__); \
	printf fmt; \
	printf("\n"); \
	putchar = temp; \
} while (0)
#else
#define debug(fmt)
#endif

#endif /* __ERROR_H__ */