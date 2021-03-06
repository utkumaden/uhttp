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
#ifndef _UHTTP_INTERNAL_
#error "This is a uHTTP internal header, don't include this file."
#endif

#ifndef _UHTTP_INTERNAL_LIST_H_
#define _UHTTP_INTERNAL_LIST_H_

#include <stddef.h>

typedef struct uhttp_list_t {
    void* head;
    size_t nsize;
    size_t nlen;
} uhttp_list_t;

/**
 * Create array list.
 * @param list List object.
 * @param nsize Size of elements.
 */
extern void uhttp_list_create(uhttp_list_t* list, size_t nsize);

/**
 * Destroy list.
 * @param list List object.
 */
extern void uhttp_list_destroy(uhttp_list_t* list);

/**
 * Append list.
 * @param list List object.
 * @param element Element to append.
 * @return Zero when successful, see errno otherwise.
 */
extern int uhttp_list_append(uhttp_list_t* list, void* element);

/**
 * Remove element from list.
 * @param list List object.
 * @param index Index to remove.
 * @return Zero when successful, see errno otherwise.
 * @remarks
 * This function may decide to temporarily leak memory. If a realloc fails then
 * the function just decrements the list size, ensuring the list object is
 * valid, meanwhile keeping the last pointer until the issue can be remedied.
 */
extern int uhttp_list_remove(uhttp_list_t* list, int index);

#define uhttp_list_index(list, type, index) (((type)*)(list)->head)[(index)]

#endif
