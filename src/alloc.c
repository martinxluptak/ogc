#include "gc_internal.h"

void *gc_alloc(size_t size)
{
    uintptr_t ptr;
    if (!(ptr = (uintptr_t) malloc(size)))
        return NULL;

    gc_ptr_t p = (gc_ptr_t){
        .start = ptr,
        .size = size,
        .marked = true,
    };
    if (__gc_object.min > ptr)
        __gc_object.min = ptr;
    if (__gc_object.max < ptr + size)
        __gc_object.max = ptr + size;


    pthread_mutex_lock(&__gc_mutex);
    gc_list_add(&__gc_object.ptr_map[HASH(ptr) % PTR_MAP_SIZE], p);
    __gc_object.ptr_num++;
    if (__gc_object.ptr_num >= __gc_object.limit)
        gc_run();
    pthread_mutex_unlock(&__gc_mutex);
    return (void *) ptr;
}

void gc_free(void *ptr)
{
    pthread_mutex_lock(&__gc_mutex);
    void *start = __gc_object.stack_start[pthread_self() % MAX_THR];
    printf("start:%p\n", start);
    gc_list_t *lst = __gc_object.ptr_map[HASH(ptr) % PTR_MAP_SIZE];
    if (lst && gc_list_exist(lst, (uintptr_t) lst)) {
        gc_list_del(&lst, (uintptr_t) lst);
        gc_mfree(lst);
    }
    pthread_mutex_unlock(&__gc_mutex);
}
