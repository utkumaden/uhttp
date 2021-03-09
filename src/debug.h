#ifndef _UHTTP_INTERNAL_
#error "This is a UHTTP internal file, don't include this."
#endif

#ifdef _DEBUG
#ifndef _UHTTP_DEBUG_H_
#define _UHTTP_DEBUG_H_

#include <stdlib.h>
#include <stdio.h>

#define uhttp_log(...) printf("\nuhttp: "__VA_ARGS__)

inline static void* uhttp_debug_malloc(size_t sz, const char* file, int line)
{
    void* ptr = malloc(sz);
    uhttp_log("malloc(%u) = %p (%s:%d)", (unsigned int)sz, ptr, file, line);
    return ptr;
}

inline static void* uhttp_debug_calloc(size_t nmemb, size_t nsz, const char* file, int line)
{
    void* ptr = calloc(nsz, nmemb);
    uhttp_log("calloc(%u, %u) = %p (%s:%d)", (unsigned int) nmemb, (unsigned int) nsz, ptr, file, line);
    return ptr;
}

inline static void* uhttp_debug_realloc(void* ptr, size_t sz, const char* file, int line)
{
    void* xptr = realloc(ptr, sz);
    uhttp_log("realloc(%p, %u) = %p (%s:%d)", ptr, (unsigned int)sz, xptr, file, line);
    return xptr;
}

inline static void uhttp_debug_free(void* ptr, const char* file, int line)
{
    free(ptr);
    uhttp_log("free(%p) (%s:%d)", ptr, file, line);
}

#define malloc(sz) uhttp_debug_malloc(sz, __FILE__ , __LINE__)
#define calloc(nmemb, nsz) uhttp_debug_calloc(nmemb, nsz, __FILE__, __LINE__)
#define realloc(ptr, sz) uhttp_debug_realloc(ptr, sz, __FILE__, __LINE__)
#define free(ptr) uhttp_debug_free(ptr, __FILE__, __LINE__)

#endif
#else
#define uhttp_log(...) 
#endif
