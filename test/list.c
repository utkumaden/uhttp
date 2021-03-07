#define _UHTTP_INTERNAL_
#include "../src/list.h"
#include "test_common.h"

#include <errno.h>
#include <limits.h>

uhttp_list_t list;

// 1
int uhttp_test_list_create_bad()
{
    // Try creating a list with a null pointer.
    // Assert:
    //  No crashes.

    uhttp_list_create(NULL, 0);

    return 1;
}

// 2
int uhttp_test_list_create_good()
{
    // Try creating a list with a normal pointer.
    // Assert:
    // nsize == sizeof(int)
    // nlen == 0

    uhttp_list_create(&list, sizeof(int));

    return list.nlen == 0 && list.nsize == sizeof(int);
}

// 3
int uhttp_test_list_append_bad_list()
{
    // Try appending a list without a pointer.
    // Assert
    // errno == EINVAL
    // retval = -1
    // xnlen == nlen
    int x = 0xDEADBEEF;
    int xnlen = list.nlen;

    return uhttp_list_append(NULL, &x) == -1 && errno == EINVAL && xnlen == list.nlen;
}

// 4
int uhttp_test_list_append_bad_element()
{
    // Try append a list with an null value.
    // Assert:
    //  errno == EINVAL
    //  retval = -1
    // xnlen == nlen
    int xnlen = list.nlen;

    return
        uhttp_list_append(&list, NULL) == -1 &&
        errno == EINVAL &&
        xnlen == list.nlen;
}

// 5
int uhttp_test_list_append_16()
{
    // Append the list with 16 values.
    // Assert:
    //  Order is preserved.
    //  nlen == 16

    for (int i = 0; i < 16; i++)
    {
        if (uhttp_list_append(&list, &i))
            return 0;
    }

    if (list.nlen != 16 || list.head == NULL)
        return 0;

    for (int *pos = list.head, i = 0; i < 16; pos++, i++)
    {
        if (*pos != i) return 0;
    }

    return 1;
}

// 6
int uhttp_test_list_remove_bad_list()
{
    // Remove from a NULL list.
    // Assert:
    // errno == EINVAL
    // retval == -1

    return uhttp_list_remove(NULL, 0) == -1 && errno == EINVAL;
}

// 7
int uhttp_test_list_remove_negative_index()
{
    // Attempt to remove negative indices
    // Assert
    // errno == EINVAL
    // retval == -1
    // xnlen == nlen
    int xnlen = list.nlen;

    return uhttp_list_remove(&list, -1) == -1 && errno == EINVAL && xnlen == list.nlen;
}

// 8
int uhttp_test_list_remove_boundary()
{
    // Attempt to remove index nlen
    // Assert
    // errno == EINVAL
    // retval == -1
    // xnlen == nlen
    int xnlen = list.nlen;

    return uhttp_list_remove(&list, list.nlen) == -1 && errno == EINVAL && xnlen == list.nlen;
}

// 9
int uhttp_test_list_remove_big_index()
{
    // Attempt to remove big indices
    // Assert
    // errno == EINVAL
    // retval == -1
    // xnlen == nlen
    int xnlen = list.nlen;

    return uhttp_list_remove(&list, INT_MAX) == -1 && errno == EINVAL && xnlen == list.nlen;
}

// 10
int uhttp_test_list_remove_odd()
{
    // Remove odd indices
    // Assert
    // retval == 0
    // nlen == 8
    // Only even values are left.
    for (int i = 0; i < 7; i++)
    {
        if (uhttp_list_remove(&list, (2 * i) + 1))
            return 0;
    }

    if (list.nlen != 8) return 0;

    for (int i = 0, *pos = list.head; i < list.nlen; i++, pos++)
    {
        if ((2 * i) != *pos) return 0;
    }

    return 1;
}

// 11
int uhttp_test_list_remove_0()
{
    // Remove first index.
    // Assert
    // retval == 0
    // xhead[0] == head[1]
    // nlen == xnlen - 1

    int xnlen = list.nlen;
    int xhead = ((int*)list.head)[1];

    if (uhttp_list_remove(&list, 0))
        return 0;

    int* pos = list.head;

    return list.nlen == xnlen - 1 && *pos == xhead;
}

// 12
int uhttp_test_list_remove_last()
{
    // Remove last index. 
    // Assert
    // retval == 0
    // nlen == xnlen - 1
    // xhead[xnlen - 2] == head[nlen - 1]

    int xnlen = list.nlen;
    int xhead = ((int*)list.head)[xnlen - 2];

    if (uhttp_list_remove(&list, xnlen - 1))
        return 0;

    int head = ((int*)list.head)[list.nlen - 1];

    return list.nlen == xnlen - 1 && xhead == head;
}

// 13
int uhttp_test_list_clear_bad_list()
{
    // Clear NULL list
    // Assert
    // retval == -1
    // errno == EINVAL

    return uhttp_list_clear(NULL) == -1 && errno == EINVAL;
}

// 14
int uhttp_test_list_clear_good()
{
    // Clear list.
    // Assert 
    // nlen == 0;
    // retval == 1

    return uhttp_list_clear(&list) == 0 && list.nlen == 0;
}

// 15
int uhttp_test_list_add_after_clear()
{
    // Add something to the list after clearing
    // Assert
    // retval == 0
    // head != NULL
    // *head == value

    int value = 0x12345678;

    return 
        uhttp_list_append(&list, &value) == 0 &&
        list.head != NULL &&
        *((int*)list.head) == value;
}

// 16
int uhttp_test_list_destroy_null()
{
    // Remove null list, ensure doesn't crash.

    uhttp_list_destroy(NULL);

    return 1;
}

int uhttp_test_list_destroy_good()
{
    // Actually destroy list ensure doesn't leak memory (somehow???)

    uhttp_list_destroy(&list);

    return 1;
}

const test_t uhttp_test_list[] = {
    { .name = "Create list with null pointer doesn't crash.", .func = uhttp_test_list_create_bad },
    { .name = "Create list with valid pointer succeeds.", .func = uhttp_test_list_create_good },
    { .name = "Append with null list fails.", .func = uhttp_test_list_append_bad_list },
    { .name = "Append with null element fails.", .func = uhttp_test_list_append_bad_element },
    { .name = "Append 16 ordered elements appears in list successfully.", .func = uhttp_test_list_append_16 },
    { .name = "Remove with null list fails.", .func = uhttp_test_list_remove_bad_list },
    { .name = "Remove with negative index fails.", .func = uhttp_test_list_remove_negative_index},
    { .name = "Remove with boundary index fails.", .func = uhttp_test_list_remove_boundary},
    { .name = "Remove with big index fails.", .func = uhttp_test_list_remove_big_index},
    { .name = "Remove with odd indices keeps even elements.", .func = uhttp_test_list_remove_odd },
    { .name = "Remove with index 0 keeps index 1 value.", .func = uhttp_test_list_remove_0 },
    { .name = "Remove with last index keeps second last value.", .func = uhttp_test_list_remove_last},
    { .name = "Clear with null list fails.", .func = uhttp_test_list_clear_bad_list},
    { .name = "Clear succeeds.", .func = uhttp_test_list_clear_good },
    { .name = "Append doesn't fail after clear.", .func = uhttp_test_list_add_after_clear},
    { .name = "Destroy null doesn't crash.", .func = uhttp_test_list_destroy_null },
    { .name = "Destroy doesn't leak (always passes???)", .func = uhttp_test_list_destroy_good},

    { .name = NULL, .func = NULL }
};

#ifdef _UHTTP_TEST_STANDALONE_
int main(int argc, char** argv)
{
    return uhttp_test_main(&uhttp_test_list);
}
#endif

