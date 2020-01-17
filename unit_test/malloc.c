#include <tester_lib.h>
#include "../src/malloc.h"

#define ALLOC_COUNT	100

OIL_TEST(null_test)
{
	OIL_ASSERT((intptr_t) NULL, (intptr_t) malloc(0));
}

OIL_TEST(linear_nofree_test)
{
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!malloc(i + 1))
			OIL_FAIL();
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_free_test)
{
	void *p;
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(p = malloc(i + 1)))
			OIL_FAIL();
		free(p);
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_list_free_test)
{
	void *arr[100];
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(i + 1)))
			OIL_FAIL();
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
	{
		free(arr[i]);
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_large_nofree_test)
{
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!malloc(512 * 1024))
			OIL_FAIL();
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_large_free_test)
{
	void *arr[ALLOC_COUNT];
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(512 * 1024)))
			OIL_FAIL();
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
	{
		free(arr[i]);
		++i;
	}
	OIL_PASS();
}

OIL_TEST(intertwined_free_test)
{
	void *arr[ALLOC_COUNT];
	size_t i = 0;

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(i + 1)))
			OIL_FAIL();
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
	{
		if(i % 2 == 0)
			free(arr[ALLOC_COUNT - i - 1]);
		else
			free(arr[i]);
		++i;
	}
	OIL_PASS();
}

void oil_prepare_tests(void)
{
	OIL_TEST_REGISTER(null_test);

	OIL_TEST_REGISTER(linear_nofree_test);
	OIL_TEST_REGISTER(linear_free_test);
	OIL_TEST_REGISTER(linear_list_free_test);
	OIL_TEST_REGISTER(linear_large_nofree_test);
	OIL_TEST_REGISTER(linear_large_free_test);

	OIL_TEST_REGISTER(intertwined_free_test);
}
