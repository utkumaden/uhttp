#ifndef _UHTTP_INTERNAL_
#error "This is an internal header file, don't include this."
#endif

#ifndef _UHTTP_TEST_COMMON_H_
#define _UHTTP_TEST_COMMON_H_

typedef struct test_t {
    const char* name;
    int (*func)();
} test_t;

extern int uhttp_test_main(const test_t* tests);

#endif
