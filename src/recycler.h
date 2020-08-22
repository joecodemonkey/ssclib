//
// Created by Joseph Hurdle on 7/5/20.
//

#ifndef SEARCHFILEC_RECYCLER_H
#define SEARCHFILEC_RECYCLER_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct stMemChunk {
    void *p;
    size_t cap;
} MemoryChunk;

typedef struct stBufferFactory {
    MemoryChunk *memory;
    size_t cap;
} Recycler;

// initialize a memory chunk [mc]
// [mc] - pointer to memory chunk to be initialized
void mem_chunk_init(MemoryChunk *mc);

// initialize a recycler structure [rc] with some sane defaults
// [rc] - recycler  to be initalized
void recycler_init(Recycler *rc);

// return memory[mem] to a recycler [rc] of length [size]
// [rc] - recycler to store the chunk
// [size] - number of bytes to return
// [mem] - bytes to return
void recycler_return(Recycler *rc, size_t size, void *mem);

// free the recycler [rc] and all chunks it currently owns
// [rc] - recycler to free
// returns buffer ptr
void recycler_free(Recycler *rc);

// get a memory chunk out of the recylcer
// [rc] - recycler to retrieve memory from
// [out] - memory chunk to populate
// [bytes] - number of bytes to allocate
// returns true on success
bool recycler_get(Recycler *rc, MemoryChunk *out, size_t bytes);

// get memory out of the recylcer [rc] of exactly [size_t] bytes or, allocate
// memory using malloc
// [rc] - recycler to retrieve memory from
// [bytes] - number of bytes to allocate
// returns exactly size_t byte allocated buffer or NULL if malloc fails
void * recycler_get_exact(Recycler *rc, size_t bytes);

#endif //SEARCHFILEC_RECYCLER_H
