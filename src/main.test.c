#include <CUnit/Basic.h>
#include <datrie/datrie.test.h>

#define TEST_SIZE(arr) (sizeof(arr) / sizeof(TestPair))

typedef struct
{
	char *label;
	void (*test)(void);
} TestPair;

typedef struct
{
	char *label;
	TestPair *suit;
	int count;
} SuitPair;

int main()
{
	// Initialize the CUnit test registry
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* DATRIE TESTS */
	TestPair datrie_tests[] = {
		{"datrie_test", datrie_test},
	};

	SuitPair tests[] = {
		{"datrie_tests", datrie_tests, TEST_SIZE(datrie_tests)},
	};

	for (int i = 0; i < sizeof(tests) / sizeof(SuitPair); i++)
	{
		CU_pSuite p_suite = CU_add_suite(tests[i].label, 0, 0);

		if (NULL == p_suite)
		{
			CU_cleanup_registry();
			return CU_get_error();
		}

		for (int j = 0; j < tests[i].count; j++)
		{
			if (NULL == CU_add_test(p_suite, tests[i].suit[j].label, tests[i].suit[j].test))
			{
				CU_cleanup_registry();
				return CU_get_error();
			}
		}
	}

	// choices are: CU_BRM_SILENT, CU_BRM_NORMAL and CU_BRM_VERBOSE
	CU_basic_set_mode(CU_BRM_NORMAL);
	CU_basic_run_tests();

	return CU_get_error();
}
