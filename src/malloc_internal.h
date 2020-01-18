#ifndef _MALLOC_INTERNAL_H
# define _MALLOC_INTERNAL_H

# include <stdint.h>
# include <string.h>

# define _FIRST_SMALL_BUCKET_SIZE	((size_t) 8)
# define _SMALL_BUCKETS_COUNT		((size_t) 6)

# define _SMALL_BIN_MAX\
	(_FIRST_SMALL_BUCKET_SIZE << (_SMALL_BUCKETS_COUNT - 1))
# define _MEDIUM_BIN_MAX	262144

# define _SMALL_BLOCK_PAGES		((size_t) 2)
# define _MEDIUM_BLOCK_PAGES	((size_t) 4)

# define ALIGNMENT	16

# define _MALLOC_CHUNK_MAGIC	(0x5ea310c36f405b33 & (sizeof(long) == 8\
	? ~((unsigned long) 0) : 0xffffffff))

# define CEIL_DIVISION(n0, n1)	((n0) / (n1) + !!((n0) % (n1)))
# define MAX(n0, n1)			((n0) >= (n1) ? (n0) : (n1))
# define DOWN_ALIGN(ptr, n)		((void *) ((uintptr_t) (ptr) & ~((n) - 1)))
// TODO Do not add `n` if ptr was already aligned
# define UP_ALIGN(ptr, n)		(DOWN_ALIGN(ptr, n) + (n))

# define BLOCK_DATA(b)		UP_ALIGN(((_block_t *) (b))->data, ALIGNMENT)
# define CHUNK_DATA(c)		UP_ALIGN(((_used_chunk_t *) (c))->data, ALIGNMENT)

# define BLOCK_HDR_SIZE(b)	((uintptr_t) (BLOCK_DATA(b) - (void *) (b)))
# define CHUNK_HDR_SIZE(c)	((uintptr_t) (CHUNK_DATA(c) - (void *) (c)))

# define GET_BLOCK(chunk)	((void *) ((void *) (chunk) - BLOCK_DATA(NULL)))
# define GET_CHUNK(ptr)		((void *) ((void *) (ptr) - CHUNK_DATA(NULL)))

typedef struct _chunk_hdr
{
	struct _chunk_hdr *prev, *next;

	size_t length;
	char used;
# ifdef _MALLOC_CHUNK_MAGIC
	long magic;
# endif
} _chunk_hdr_t;

typedef struct _used_chunk
{
	_chunk_hdr_t hdr;
	char data[0];
} _used_chunk_t;

typedef struct _free_chunk
{
	_chunk_hdr_t hdr;
	struct _free_chunk *prev_free, *next_free;
} _free_chunk_t;

typedef struct _block
{
	struct _block *prev, *next;

	size_t pages;
	char data[0];
} _block_t;

void *_alloc_pages(size_t n);
void _free_pages(void *addr, size_t n);

_block_t *_alloc_block(const size_t pages);
void _free_block(_block_t *b);

void _bucket_link(_free_chunk_t *chunk);
void _bucket_unlink(_free_chunk_t *chunk);

void *_small_alloc(size_t size);
void *_medium_alloc(size_t size);
void *_large_alloc(size_t size);

#endif
