//
// Created by Joseph Hurdle on 7/14/20.
//



#include <assert.h>
#include "buffer.h"
#include "recycler.h"
#include "bufferarray.h"
#include "log.h"
#include <string.h>

size_t buffer_array_get_buffer_count(BufferArray *ba) {
    assert(NULL != ba);
    return ba->count;
}

size_t buffer_array_get_capacity(BufferArray *ba) {
    assert(NULL != ba);
    if(NULL == ba->array.data) return 0;
    return buffer_get_capacity(&ba->array) / sizeof(Buffer);
}

void buffer_array_init(BufferArray *ba) {
    assert(NULL != ba);
    buffer_init(&ba->array);
    ba->count = 0;
    ba->recycler = NULL;
}

Buffer * buffer_array_get_buffer(BufferArray *ba, size_t off)
{
    assert(NULL != ba);
    if(ba->count <= off) return NULL;
    return (Buffer *) (&ba->array.data[off * sizeof(Buffer)]);
}

bool buffer_array_push(BufferArray *ba, const Buffer *buf) {
    assert(NULL != ba);
    assert(NULL != buf);

    Buffer *tmp;

    tmp = malloc(sizeof(Buffer));

    if(NULL == tmp) {
        log_message("Unable to allocate %zu bytes to hold buffer", sizeof(Buffer));
        return false;
    }

    buffer_init(tmp);
    tmp->recycler = ba->recycler;

    if(!buffer_cpy(tmp, buf)) {
        log_message("Unable to copy buffer");
        return false;
    }

    if(!buffer_push_bytes(&ba->array, (unsigned char *) tmp, sizeof(Buffer))) {
        log_message("Unable to add buffer to array");
        return false;
    }

    ba->count++;

    return true;
}

bool buffer_array_cpy_buffer(BufferArray *ba, size_t idx, Buffer *out) {
    assert(NULL != ba);
    assert(NULL != out);
    if(idx >= ba->count) return false;

    buffer_cpy(out, buffer_array_get_buffer(ba, idx));
    return true;
}

void buffer_array_remove_buffer(BufferArray * ba, size_t idx)
{
    assert(NULL != ba);
    if(idx >= ba->count) {
        log_message("attempt to remove buffer at idx[%zu] past"
            "length of buffer array %zu", idx, ba->count);
        return;
    }
    if(NULL == ba->array.data) {
        log_message("attempt to remove buffer from empty buffer array");
        return;
    }

    Buffer * old = buffer_array_get_buffer(ba, idx);
    buffer_free(old);

    Buffer *moveStart = buffer_array_get_buffer(ba, idx + 1);
    // bytes to move is

    // [ ] [ ] [ ] 3 buffers
    // drop buffer idx 1
    // [ ] [x] [ ]
    // buffers to move is addr of last buffer (2) - addr of deleted (1) = 1
    // drop buffer idx 2
    // buffers to move is addr of last buffer (2) - addr of deleted (2) = 0
    // drop buffer idx 0
    // buffers to move is addr of last buffer (2) - add of deleted (0) = 2

    size_t buffersToMove = buffer_array_get_buffer_count(ba) - 1 - idx;
    size_t memToMove = buffersToMove + sizeof(Buffer);

    memmove(old, moveStart, memToMove);

    ba->count -= 1;
}

void buffer_array_assign_recycler(BufferArray *ba, Recycler *rc) {
    assert(NULL != ba);
    ba->recycler = rc;
    ba->array.recycler = rc;
}


void buffer_array_free(BufferArray *ba) {
    assert(NULL != ba);
    Recycler * r = ba->recycler;

    if(buffer_is_empty(&ba->array)) return;

    for(size_t i=0; i< ba->count; ++i) {
        Buffer * cur = buffer_array_get_buffer(ba, i);
        buffer_free(cur);
    }

    buffer_free(&ba->array);
    buffer_array_init(ba);
    ba->recycler = r;
}

void buffer_array_clone(BufferArray *dest, BufferArray *src) {
    assert(NULL != dest);
    assert(NULL != src);
    dest->recycler = src->recycler;
    dest->count = src->count;
    buffer_clone(&dest->array, &src->array);
}