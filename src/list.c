/**
 * Copyright 2021 H. Utku Maden
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#define _UHTTP_INTERNAL_
#include "list.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

void uhttp_list_create(uhttp_list_t* list, size_t nsize)
{
    if (list)
    {
        list->head = NULL;
        list->nlen = 0;
        list->nsize = nsize;
    }
}

void uhttp_list_destroy(uhttp_list_t* list)
{
    if (list)
    {
        free(list->head);
    }
}

int uhttp_list_append(uhttp_list_t* list, const void* element)
{
    if (list == NULL || element == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    void* xhead = realloc(list->head, (list->nlen + 1) * list->nsize);
    if (!xhead)
    {
        errno = ENOMEM;
        return -1;
    }

    list->head = xhead;
    memcpy((char*)list->head + ((list->nlen++) * list->nsize), element, list->nsize);

    return 0;
}

int uhttp_list_remove(uhttp_list_t* list, int index)
{
    if (list == NULL || index < 0 || index >= list->nlen)
    {
        errno = EINVAL;
        return -1;
    }

    // Overwrite old element.
    memmove(
        (char*)list->head + (list->nsize * index),         // dest
        (char*)list->head + (list->nsize * (1 + index)),   // src
        (list->nlen - index - 1) * list->nsize             // count
    );

    // Downsize List.
    void* xhead = realloc(list->head, (--list->nlen) * list->nsize);
    if (xhead || list->nlen == 0)
    {
        // Only write head to list if it is a valid pointer, it is ok to
        // temporarily leak it, as the next realloc will fix the issue.
        // It can also be NULL so long as nlen == 0.
        list->head = xhead;
    }

    return 0;
}

int uhttp_list_clear(uhttp_list_t* list)
{
    if (list == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    free(list->head);
    list->head = NULL;
    list->nlen = 0;

    return 0;
}
