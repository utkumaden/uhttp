#define _UHTTP_INTERNAL_
#include "test_common.h"

#include <stdio.h>
#include <time.h>

int uhttp_test_main(const test_t* tests)
{
    clock_t clk_s = clock(), clk_e;

    printf("Unit Test Start. %d\n", clk_s);

    int i = 1;
    int result = 1;
    for (test_t* test = tests; test->name != NULL; test++, i++)
    {
        printf("[%d] %s: ", i, test->name);

        result = test->func();

        printf("%s\n", result ? "PASS" : "FAIL");

        if (result == 0)
        {
            break;
        }
    }

    clk_e = clock();

    if (result == 0)
    {
        printf("Unit test stoppped early.\n");
    }
    printf("Unit Test End (%d tests). %d (%f ms)\n", i, clk_e, (float)(clk_e - clk_s) * 1000.0f / CLOCKS_PER_SEC);

    return !result;
}
