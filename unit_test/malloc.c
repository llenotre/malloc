#include <tester_lib.h>
#include "../src/malloc.h"

#define ALLOC_COUNT	100

OIL_TEST(null_test)
{
	OIL_ASSERT(NULL, malloc(0));
}

OIL_TEST(linear_nofree_test)
{
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!malloc(i + 1))
			OIL_ASSERT(0, 1);
		++i;
	}
	OIL_ASSERT(1, 1);
}

OIL_TEST(linear_free_test)
{
	void *p;
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(p = malloc(i + 1)))
			OIL_ASSERT(0, 1);
		free(p);
		++i;
	}
	OIL_ASSERT(1, 1);
}

OIL_TEST(linear_list_free_test)
{
	void *arr[100];
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(i + 1)))
			OIL_ASSERT(1, 0);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
	{
		free(arr[i]);
		++i;
	}
	OIL_ASSERT(1, 1);
}

OIL_TEST(linear_large_nofree_test)
{
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!malloc(512 * 1024))
			OIL_ASSERT(0, 1);
		++i;
	}
	OIL_ASSERT(1, 1);
}

OIL_TEST(linear_large_free_test)
{
	void *arr[ALLOC_COUNT];
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(512 * 1024)))
			OIL_ASSERT(0, 1);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
	{
		free(arr[i]);
		++i;
	}
	OIL_ASSERT(1, 1);
}

void oil_prepare_tests(void)
{
	OIL_TEST_REGISTER(null_test);

	OIL_TEST_REGISTER(linear_nofree_test);
	OIL_TEST_REGISTER(linear_free_test);
	OIL_TEST_REGISTER(linear_list_free_test);
	OIL_TEST_REGISTER(linear_large_nofree_test);
	OIL_TEST_REGISTER(linear_large_free_test);
}
