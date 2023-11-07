#ifndef ARRAY_H_
#define ARRAY_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// old implementation
//#define array_meta(array)
//    (&CAST(Array*, (array))[-1])

/*
 * *(arr + i) == arr[i]
 */

typedef struct {
    size_t size;
    size_t capacity;
} Array;

#define ASSERT(expression) \
    CAST(void, (expression) ? CAST(void, 0) : report_assertion(#expression, __FILE__, __LINE__))

#define CAST(T, expr) ((T)(expr))

#define array_meta(array) \
    (CAST(Array*, (array)) - 1)

#define array_try_grow(array, size_) \
    (((array) && array_meta(array)->size + (size_) < array_meta(array)->capacity) \
     ? true \
     : array_grow(CAST(void **, &(array)), (size_), sizeof *(array)))

#define array_size(array) \
    ((array) ? array_meta(array)->size : 0)

#define array_capacity(array) \
    ((array) ? array_meta(array)->capacity : 0)

#define array_expand(array, size_) \
    (array_try_grow((array), (size_)) \
     ? (array_meta(array)->size += (size_), true) \
     : false)

#define array_push(array, value) \
    (array_try_grow((array), 1) \
     ? ((array)[array_meta(array)->size++] = (value), true) \
     : false)

#define array_pop_front(array) \
    (array_size(array) ? \
     (memmove((array), &(array)[1], sizeof *(array) * (array_meta(array)->size - 1)), \
      array_meta(array)->size--, true) \
     : false)

#define array_pop_last(array) \
    (array_size(array) ? \
     (array_resize((array), array_size(array) - 1), true) \
     : false)

#define array_pop_at(array, index) \
    (array_size(array) && (index) < array_size(array) ?	\
     (memmove((array) + (index), &(array)[(index) + 1], sizeof *(array) * (array_meta(array)->size - (index) - 1)), \
      array_meta(array)->size--, true) \
     : false)

#define array_free(array) \
    array_delete((void*)array)

#define array_resize(array, size_) \
    ((array) \
     ? (array_meta(array)->size >= (size_) \
	? (array_meta(array)->size = (size_), true) \
	: array_expand((array), (size_) - array_meta(array)->size)) \
     : (array_grow(CAST(void **, &(array)), (size_), sizeof *(array)) \
	? (array_meta(array)->size = (size_), true) \
	: false))

#define array_last(array) \
    ((array)[array_size(array) - 1])

#define array_clear(array) \
    (void)((array) ? array_meta(array)->size = 0 : 0)

#define array_df(X) X* __attribute__((cleanup(array_defer)))

__attribute__((__noreturn__)) void report_assertion(const char *expression, const char *file, int line);
bool array_grow(void **const array, size_t elements, size_t type_size);
void array_delete(void **const array);
void *array_create_init(size_t capacity, size_t type_size);

#ifdef ARRAY_IMPLEMENTATION

__attribute__((unused)) static void array_defer(void *array) {
    array_free(*(void**)array);
}

__attribute__((__noreturn__)) void report_assertion(const char *expression, const char *file, int line) {
    fprintf(stderr, "%s:%d: Assertion failed on %s\n", file, line, expression);
    abort();
}

bool array_grow(void **const array, size_t elements, size_t type_size) {
    ASSERT(*array);
    Array *meta = array_meta(*array);
    const size_t count = 2 * meta->capacity + elements;
    void *data = realloc(meta, type_size * count + sizeof *meta);
    if (!data) {
	free(meta);
	return false;
    }
    meta = CAST(Array *, data);
    meta->capacity = count;
    *array = meta + 1;
    return true;
}

void array_delete(void **const array) {
    ASSERT(array);
    Array *const meta = array_meta(array);
    free(meta);
}

void *array_create_init(size_t capacity, size_t type_size) {
    void *data = malloc(type_size * capacity + sizeof(Array));
    if (!data) {
	return 0;
    }
    Array *array = CAST(Array *, data);
    array->size = 0;
    array->capacity = capacity;
    return array + 1;
}

#endif // ARRAY_IMPLEMENTATION

#endif // ARRAY_H_
