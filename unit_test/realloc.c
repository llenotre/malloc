#include <tester_lib.h>
#include "../src/malloc.h"

OIL_TEST(realloc_alloc)
{
	void *p;

	if(!(p = realloc(NULL, 8)))
		OIL_FAIL();
	free(p);
	OIL_PASS();
}

OIL_TEST(realloc_free)
{
	void *p;

	if(!(p = malloc(8)))
		OIL_FAIL();
	(void) realloc(p, 0);
	OIL_PASS();
}

OIL_TEST(realloc_grow)
{
	size_t i = 1;
	void *p = NULL;

	do
	{
		if(!(p = realloc(p, i)))
			OIL_FAIL();
		i *= 2;
	}
	while(i <= 32 * 1024 * 1024);
	free(p);
	OIL_PASS();
}

OIL_TEST(realloc_shrink)
{
	size_t i = 32 * 1024 * 1024;
	void *p = NULL;

	do
	{
		if(!(p = realloc(p, i)))
			OIL_FAIL();
		i /= 2;
	}
	while(i > 0);
	free(p);
	OIL_PASS();
}

void oil_prepare_tests(void)
{
	OIL_TEST_REGISTER(realloc_alloc);
	OIL_TEST_REGISTER(realloc_free);

	OIL_TEST_REGISTER(realloc_grow);
	OIL_TEST_REGISTER(realloc_shrink);
}
