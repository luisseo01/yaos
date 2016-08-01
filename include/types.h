#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL				((void *)0)
#define EOF				(-1)

#define HIGH				1
#define ON				1
#define ENABLE				1

#define LOW				0
#define OFF				0
#define DISABLE				0

#define UNLOCKED			1

typedef enum {false = 0, true = 1} bool;
typedef unsigned short int refcnt_t;
typedef unsigned short int mode_t;
typedef volatile int lock_t;
typedef unsigned int dev_t;
typedef unsigned int size_t;
typedef unsigned long long uint64_t;
struct list;
typedef struct list buf_t;

#define WORD_SIZE			sizeof(int)
#define WORD_BITS			(WORD_SIZE << 3)

#define max(a, b)			(((a) > (b))? a : b)
#define min(a, b)			(((a) > (b))? b : a)

#define BASE_WORD(x)			((unsigned int)(x) & ~(WORD_SIZE-1))
#define ALIGN_WORD(x)			\
	BASE_WORD((unsigned int)(x) + (WORD_SIZE-1))
#define BASE_DWORD(x)			\
	((unsigned int)(x) & ~((WORD_SIZE << 1) - 1))
#define ALIGN_DWORD(x)			\
	BASE_DWORD((unsigned int)(x) + ((WORD_SIZE << 1) - 1))
#define BLOCK_BASE(x, size)		\
	((unsigned int)(x) & ~((size)-1))
#define ALIGN_BLOCK(x, size)		\
	BLOCK_BASE((unsigned int)(x) + ((size)-1), size)

#define get_container_of(ptr, type, member) \
	((type *)((char *)ptr - (char *)&((type *)0)->member))

/* double linked list */
struct list {
	struct list *next, *prev;
};

#define INIT_LIST_HEAD(name) 		{ &(name), &(name) }
#define DEFINE_LIST_HEAD(name)		\
	struct list name = INIT_LIST_HEAD(name)

static inline void list_link_init(struct list *list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list *new, struct list *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

static inline void list_del(struct list *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline int list_empty(const struct list *node)
{
	return (node->next == node) && (node->prev == node);
}

/* fifo */
struct fifo {
	size_t size;
	unsigned int front, rear;
	void *buf;
};

extern void fifo_init(struct fifo *q, void *queue, size_t size);
extern int  fifo_get(struct fifo *q, int type_size);
extern int  fifo_put(struct fifo *q, int value, int type_size);
extern void fifo_flush(struct fifo *q);

#define SWAP_WORD(word)	\
		((word >> 24) | (word << 24) | ((word >> 8) & 0xff00) | \
		 ((word << 8) & 0xff0000))

#endif /* __TYPES_H__ */
