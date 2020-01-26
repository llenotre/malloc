#ifndef _MALLOC_INTERNAL_H
# define _MALLOC_INTERNAL_H

# include <stdint.h>
# include <string.h>

# define _FIRST_SMALL_BUCKET_SIZE	((size_t) 8)
# define _SMALL_BUCKETS_COUNT		((size_t) 6)

# define _SMALL_BIN_MAX\
	(_FIRST_SMALL_BUCKET_SIZE << (_SMALL_BUCKETS_COUNT - 1))

# define _FIRST_MEDIUM_BUCKET_SIZE	_SMALL_BIN_MAX
# define _MEDIUM_BUCKETS_COUNT		((size_t) 11)

# define _MEDIUM_BIN_MAX			((size_t) 262144)

# define _SMALL_BLOCK_PAGES		((size_t) 8)
# define _MEDIUM_BLOCK_PAGES	((size_t) 128)

/*
 * Chunks alignment boundary.
 */
# define ALIGNMENT				16
/*
 * Magic number used to check integrity of memory chunks.
 */
# define _MALLOC_CHUNK_MAGIC	(0x5ea310c36f405b33 & (sizeof(long) == 8\
	? ~((unsigned long) 0) : 0xffffffff))

/*
 * Performs `ceil(n0 / n1)` without using floating point numbers
 */
# define CEIL_DIVISION(n0, n1)	((n0) / (n1) + !!((n0) % (n1)))
/*
 * Checks if the given pointer is aligned to the given boundary.
 */
# define IS_ALIGNED(ptr, n)		(((uintptr_t) (ptr) & ((n) - 1)) == 0)
# define DOWN_ALIGN(ptr, n)		((void *) ((uintptr_t) (ptr) & ~((n) - 1)))
# define UP_ALIGN(ptr, n)\
	(IS_ALIGNED(ptr, n) ? (ptr) : DOWN_ALIGN(ptr, n) + (n))

/*
 * Returns the lowest between the two given values.
 */
# define MIN(n0, n1)			((n0) <= (n1) ? (n0) : (n1))
/*
 * Returns the greatest between the two given values.
 */
# define MAX(n0, n1)			((n0) >= (n1) ? (n0) : (n1))

# define BLOCK_DATA(b)		UP_ALIGN(((_block_t *) (b))->data, ALIGNMENT)
# define CHUNK_DATA(c)		UP_ALIGN(((_used_chunk_t *) (c))->data, ALIGNMENT)

# define BLOCK_HDR_SIZE		((size_t) BLOCK_DATA(NULL))
# define CHUNK_HDR_SIZE		((size_t) CHUNK_DATA(NULL))

# define GET_CHUNK(ptr)		((void *) ((void *) (ptr) - CHUNK_DATA(NULL)))

# define likely(n)			__builtin_expect(!!(n), 1)
# define unlikely(n)		__builtin_expect(!!(n), 0)

typedef struct _block _block_t;

/*
 * Memory chunk header
 */
typedef struct _chunk_hdr
{
	struct _chunk_hdr *prev, *next;

	_block_t *block;
	size_t length;
	char used;
# ifdef _MALLOC_CHUNK_MAGIC
	long magic;
# endif
} _chunk_hdr_t;

/*
 * Used chunk structure
 */
typedef struct _used_chunk
{
	_chunk_hdr_t hdr;
	char data[0];
} _used_chunk_t;

/*
 * Free chunk structure
 */
typedef struct _free_chunk
{
	_chunk_hdr_t hdr;
	struct _free_chunk *prev_free, *next_free;
} _free_chunk_t;

/*
 * Memory block structure
 */
typedef struct _block
{
	struct _block *prev, *next;

	size_t pages;
	char data[0];
} _block_t;

size_t _get_page_size(void);
void *_alloc_pages(size_t n);
void _free_pages(void *addr, size_t n);

_block_t *_alloc_block(const size_t pages);
_block_t **_block_get_bin(_block_t *b);
void _free_block(_block_t *b);

void _bucket_link(_free_chunk_t *chunk);
void _bucket_unlink(_free_chunk_t *chunk);
_free_chunk_t **_get_bucket(size_t size, int insert, int medium);

void _split_chunk(_chunk_hdr_t *chunk, size_t size);
void _merge_chunks(_chunk_hdr_t *c);
void _alloc_chunk(_free_chunk_t *chunk, size_t size);

void *_small_alloc(size_t size);
void *_medium_alloc(size_t size);
void *_large_alloc(size_t size);

void _chunk_assert(_chunk_hdr_t *c);

# ifdef _MALLOC_DEBUG
void _debug_print_malloc_info(void);
void _debug_show_alloc(void);
void _debug_global_check(void);
# endif

#endif
