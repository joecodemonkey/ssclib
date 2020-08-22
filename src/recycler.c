//
// Created by Joseph Hurdle on 7/5/20.
//

#include <assert.h>
#include "recycler.h"
#include "string.h"
#include "log.h"
#include "hashtable.h"
#include "buffer.h"

// memory chunk compare for recylcer sorting
// compares using capacity
int mem_chunk_compare(void *a, void *b) {

    MemoryChunk *ptrA = (MemoryChunk* ) a;
    MemoryChunk *ptrB = (MemoryChunk* ) b;
    if(NULL == ptrB->p) return 1;
    if(NULL == ptrA->p) return -1;
    return (int) ptrA->cap - (int) ptrB->cap;
}

void mem_chunk_init(MemoryChunk *mc) {
    assert(NULL != mc);

    mc->cap = 0;
    mc->p = NULL;
}


// expand recycler [rc] by CAPACITY_INCREMENT bytes
// so that it can hold more chunks
bool recycler_expand(Recycler *rc) {

    assert(NULL != rc);

    MemoryChunk *temp = rc->memory;
    const size_t newLen = rc->cap + CAPACITY_INCREMENT;

    rc->memory = malloc(sizeof(MemoryChunk) * newLen);

    if(NULL == rc->memory) {
        rc->memory = temp; // don't lose the memory (if it is even there)
        log_message("Unable to expand recycler capacity to %zu", newLen);
        return false;
    }

    memcpy(rc->memory, temp, sizeof(MemoryChunk) * rc->cap);
    for(size_t i = rc->cap; i < newLen; ++i) {
        mem_chunk_init(&rc->memory[i]);
    }
    rc->cap = newLen;

    return true;
}

void recycler_return(Recycler *rc, size_t size, void *mem) {

    assert(NULL != rc);
    assert(NULL != mem);

    for(int i=0; i < rc->cap; ++i) {

        MemoryChunk *ptr = (MemoryChunk *) &rc->memory[i];
        if(ptr->cap == 0 || NULL == ptr->p) {
            ptr->p = mem;
            ptr->cap = size;
            mem_chunk_init(mem);
            return;
        }
    }

    if(!recycler_expand(rc)) {
        free(mem);
        mem_chunk_init(mem);
    }
    else return recycler_return(rc, size, mem);
}

void recycler_free(Recycler *rc) {

    assert(NULL != rc);

    if(NULL == rc->memory) return;

    if(NULL != rc->memory) {
        for (int i = 0; i < rc->cap; ++i) {
            if (NULL != rc->memory[i].p) free(rc->memory[i].p);
        }
        free(rc->memory);
    }

    recycler_init(rc);
}

bool recycler_get(Recycler *rc, MemoryChunk *out, size_t bytes) {

    assert(NULL != rc);
    assert(NULL != out);

    MemoryChunk *ptr;

    for(size_t i=0; i<rc->cap; ++i) {

        ptr = &rc->memory[i];
        if(ptr->cap >= bytes && NULL != ptr->p) {
            out->p = ptr->p;
            out->cap = ptr->cap;
            ptr->p = NULL;
            ptr->cap = 0;
            return true;
        }
    }

    out->p = malloc(bytes);
    if(NULL == out->p) {
        log_message("failure allocated %zu bytes", bytes);
        return false;
    }
    out->cap = bytes;

    return true;
}

void * recycler_get_exact(Recycler *rc, size_t bytes) {
    assert(NULL != rc);
    void * ret = NULL;

    for(size_t i=0; i<rc->cap; ++i) {

        MemoryChunk * tmp = &rc->memory[i];
        if(tmp->cap == bytes && NULL != tmp->p) {
            ret = tmp->p;
            tmp->p = NULL;
            tmp->cap = 0;
            break;
        }
    }
    if (NULL == ret) {
        ret = malloc(bytes);
        if(NULL == ret) {
            log_message("unable to allocate %zu bytes", bytes);
        }
    }

    return ret;
}

void recycler_init(Recycler *rc) {
    rc->cap = 0;
    rc->memory = NULL;
}