#ifndef _MALLOC_H
# define _MALLOC_H

# include <string.h>

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);

# ifdef _MALLOC_DEBUG
void _debug_show_alloc(void);
# endif

#endif
