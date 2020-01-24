#ifndef _MALLOC_H
# define _MALLOC_H

/*
 * If _MALLOC_CONFLICT is defined, changes the name of functions to prevent name
 * conflicts (allows to prevent stackoverflow with dprintf).
 */
# ifdef _MALLOC_CONFLICT
#  define malloc(...)		_malloc(__VA_ARGS__)
#  define calloc(...)		_calloc(__VA_ARGS__)
#  define realloc(...)		_realloc(__VA_ARGS__)
#  define reallocarray(...)	_reallocarray(__VA_ARGS__)
#  define free(...)			_free(__VA_ARGS__)
# endif

# include <string.h>

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);

#endif
