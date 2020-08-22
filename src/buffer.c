//
// Created by Joseph Hurdle on 7/4/20.
//

#include <stdbool.h>
#include <assert.h>
#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include "recycler.h"
#include <ctype.h>
#include <stdio.h>
#include "log.h"

void buffer_to_memchunk(Buffer *buf, MemoryChunk *mc) {
    assert(NULL != buf);
    assert(NULL != mc);
    mc->cap = buf->cap;
    mc->p = buf->data;
}

bool buffer_push_null(Buffer *dest) {

    bool allocated = false;

    if (!buffer_reserve(dest, dest->len + 1)) {
        log_message("Unable to expand buffer to hold %zu bytes.", dest->len + 1);
        return false;
    }

    dest->data[dest->len] = 0;

    dest->len++;

    return true;
}

bool buffer_is_empty(const Buffer *buf) {
    assert(NULL != buf);
    return (buf->len == 0 || buf->cap == 0 || buf->data == NULL);
}

void buffer_init(Buffer *buf) {
    buf->len = buf->cap = 0;
    buf->data = NULL;
    buf->nullTerminated = false;
    buf->recycler = NULL;
}

void buffer_swap(Buffer *a, Buffer *b) {

    assert(NULL !=a);
    assert(NULL != b);

    Buffer tmp;

    tmp.data = a->data;
    tmp.len = a->len;
    tmp.cap = a->cap;
    tmp.recycler = a->recycler;

    a->data = b->data;
    a->len = b->len;
    a->cap = b->cap;
    a->nullTerminated = b->nullTerminated;
    a->recycler = b->recycler;

    b->data = tmp.data;
    b->len = tmp.len;
    b->cap = tmp.cap;
    b->nullTerminated = tmp.nullTerminated;
    b->recycler = tmp.recycler;
}

int buffer_cmp(const void *a, const void*b) {
    // try to get away with just using length
    long long ret = (long long) ((Buffer *) a)->len - (long long) ((Buffer *) b)->len;
    if (ret != 0) return (int) ret;
    return memcmp(((Buffer *) a)->data,
                  ((Buffer *) b)->data, ((Buffer *) a)->len);
}

bool buffer_reserve(Buffer *buf, unsigned int bytes) {

    assert(NULL != buf);

    if(buf->cap >= bytes) return true;

    bool allocated = false;

    MemoryChunk chunk;
    MemoryChunk tmp;

    mem_chunk_init(&chunk);
    mem_chunk_init(&tmp);

    if(NULL != buf->recycler) {
        allocated = recycler_get(buf->recycler, &chunk, bytes);
        if(!allocated) chunk.p = NULL;
    }
    else {
        chunk.p = malloc(bytes);
        if(NULL != chunk.p) chunk.cap = bytes;
    }

    if(NULL == chunk.p) {
        log_message("failure to reserve %d bytes", bytes);
        return false;
    }

    if(NULL == buf->data || 0 == buf->cap) {
        buf->data = chunk.p;
        buf->cap = chunk.cap;
        return true;
    }

    if(buf->len > 0) {
        memcpy(chunk.p, buf->data, buf->len);
    }

    tmp.p = buf->data;
    tmp.cap = buf->cap;
    buf->data = chunk.p;
    buf->cap = chunk.cap;

    if(NULL != buf->recycler) {
        recycler_return(buf->recycler, tmp.cap, tmp.p);
    }
    else free(tmp.p);

    return true;
}

void buffer_free(Buffer *buf)
{
    Recycler * r = buf->recycler;
    if(NULL != buf->data && 0 != buf->cap) {
        if(NULL != r) {
            recycler_return(buf->recycler, buf->cap, buf->data);
        }
        else free(buf->data);

    }
    buffer_init(buf);
    buf->recycler = r;
}


bool buffer_cpy(Buffer *dest, const Buffer *src) {

    assert(NULL != dest);
    assert(NULL != src);

    bool allocated = false;

    if(0 == src->len || NULL == src->data) {
        dest->len = 0;
        return true;
    }

    if(!buffer_reserve(dest, src->len)) {
        log_message("Unable to expand dest buffer to hold %d bytes", src->len);
        return false;
    }

    memcpy(dest->data, src->data, src->len);
    dest->len = src->len;
    dest->nullTerminated = src->nullTerminated;

    return true;
}

void buffer_clone(Buffer *dest, const Buffer *src) {

    assert(NULL != dest);
    assert(NULL != src);

    dest->data = src->data;
    dest->len = src->len;
    dest->cap = src->cap;
    dest->nullTerminated = src->nullTerminated;
    dest->recycler = src->recycler;
}


bool buffer_append(Buffer *dest, const Buffer *src) {

    assert(NULL != dest);
    assert(NULL != src);

    if(0 == src->len || NULL == src->data) return true;

    const size_t reqLen = dest->len + src->len;

    if(!buffer_reserve(dest, reqLen)) {
        log_message("Unable to expand buffer to hold %d bytes.", reqLen);
        return false;
    }

    memcpy(&dest->data[dest->len], src->data, src->len);
    dest->len = reqLen;
    return true;
}


size_t buffer_get_freespace(const Buffer *buf) {
assert(NULL != buf);
return buf->cap - buf->len;
}

bool buffer_push_byte(Buffer *dest, unsigned char c) {

    bool allocated = false;

    if (!buffer_reserve(dest, dest->len + 1)) {
        log_message("Unable to expand buffer to hold %zu bytes.", dest->len + 1);
        return false;
    }
    if (dest->nullTerminated) {
        if(0 == dest->data[dest->len - 1])
            dest->data[dest->len - 1] = c;
        else dest->data[dest->len++] = c;
        return buffer_push_null(dest);
    }

    dest->data[dest->len++] = c;

    return true;
}

void buffer_clear(Buffer *buf) {

    assert(NULL != buf);
    buf->len = 0;
}

bool buffer_strcpy(Buffer *buf, const char *str) {

    assert(NULL != buf);
    assert(NULL != str);

    const unsigned int len = strlen(str) + 1;

    if(!buffer_reserve(buf, len)) {
        log_message("Unable to expand buffer by %zu bytes.", len);
        return false;
    }

    memcpy(buf->data, str, len);

    buf->len = len;

    return buffer_make_string(buf);
}

bool buffer_make_string(Buffer *buf) {

    assert(NULL != buf);

    if(buf->nullTerminated) {
        if(buf->data[buf->len - 1] == 0) return true;
    }

    bool ret = buffer_push_null(buf);
    if(!ret) {
        log_message("Unable to push a null onto buffer");
        return false;
    }
    buf->nullTerminated = true;
    return true;
}

bool buffer_lshift(Buffer *buf, size_t off) {
    assert(NULL !=  buf);

    if(off > buf->len)
    {
        log_message("Requested shift offset %zu greater than data->len %zu",
                    off, buf->len);
        return false;
    }
    if(off == buf->len) {
        buf->len = 0;
        return true;
    }

    buf->len = buf->len - off;

    memmove(buf->data, &buf->data[off], buf->len);
    return true;
}

void buffer_trim_suffix(Buffer *buf, const char *suffix) {

    assert(NULL != buf);
    assert(NULL != suffix);

    const size_t len = strlen(suffix);

    if(buffer_is_empty(buf) || buf->len < len || NULL == buf->data) return;

    int ret;
    if(buf->nullTerminated)
    {
        ret = memcmp(&buf->data[buf->len - len - 1], suffix, len);
        if(0 == ret) {
            buf->len -= (len + 1);
            buffer_make_string(buf); // guaranteed to succeed
        }

    } else {
         ret = memcmp(&buf->data[buf->len - len], suffix, len);
         if(0 == ret) {
             buf->len -= (len);
         }
    }
}

void buffer_downcase(Buffer *line) {

    assert(line != NULL);

    if(NULL == line->data) return;
    if(0 == line->len) return;
    if(0 == line->cap) return;

    unsigned char *c;
    const unsigned char *end = line->data + line->len;

    for(c=line->data; c < end; ++c) if(isalpha(*c)) *c = tolower(*c);
}

void buffer_cleanse_text(Buffer *line) {

    assert(NULL != line);
    if(0 == line->len) return;
    if(0 == line->cap) return;

    // pointer to current byte in string
    unsigned char *c;

    // pointer to current position in string we wish to write
    unsigned char *d= line->data;

    for(unsigned long long off = 0; off < line->len; ++off) {
        // set to the current byte in string
        c = &line->data[off];
        // if the current byte is not alphanumeric or a blank, skip it
        if(!isalpha(*c) && !isdigit(*c) && *c != ' ') {
            continue;
        }
        else {
            // if the current digit is a blank and we are not
            // looking at the first digit in the buffer
            if(' ' == *c && off > 0) {
                // if the previous digit was a blank also
                if(*(c - 1) == ' ') {
                    // skip this byte, ignoring double (or triple) blanks
                    continue;
                }
            }
            // assign the digit to d
            *d = *c;
            // increment d
            d++;
        }
    }
    // the length of the buffer is now the original length
    // minus the difference between c and d (c will be greater if
    // anything was skipped (or 0)
    line->len = line->len - (c - d);
    if(line->nullTerminated) line->data[line->len - 1] = 0;
}


bool buffer_split(const Buffer *src, unsigned char delim, BufferArray *out)
{
    assert(NULL != src);
    assert(NULL != out);

    buffer_array_init(out);
    out->recycler = src->recycler;

    if(NULL == src->data) return false;
    if(buffer_is_empty(src)) return false;

    const unsigned char * end = src->data + src->len;
    unsigned char *p = src->data;

    Buffer token;
    buffer_init(&token);
    buffer_assign_recycler(&token, src->recycler);

    bool tokenDone = false;
    bool done = false;

    while(!done) {

        tokenDone = false;

        if (end == p) { // we are off the ned of the loop

            if (!buffer_is_empty(&token)) tokenDone = true;
            done = true;
        } else if (*p == delim) {
            if (!buffer_is_empty(&token)) tokenDone = true;
        }
        else {
            buffer_push_byte(&token, *p);
        }

        if (tokenDone) {
            if(buffer_is_null_terminated(src)) {
                if(!buffer_make_string(&token)) {
                    log_message("Unable to make token a string");
                    recycler_return(src->recycler, token.cap, token.data);
                    return false;
                }
            }
            if (!buffer_array_push(out, &token)) {
                log_message("Unable to add token to output bufferarray");
                buffer_array_free(out);

                MemoryChunk temp;
                buffer_to_memchunk(&token, &temp);

                if(src->recycler) {
                    recycler_return(src->recycler, temp.cap, temp.p);
                } else {
                    free(temp.p);
                }
                return false;
            }
            buffer_init(&token);
            buffer_assign_recycler(&token, src->recycler);
        }

        ++p;

    }

    return true;
}

size_t buffer_get_capacity(const Buffer *src) {
    assert(NULL != src);
    if(NULL == src->data) return 0;
    return src->cap - src->nullTerminated;
}

size_t buffer_get_size(const Buffer *src) {
    assert(NULL != src);
    if(NULL == src->data || 0 == src->cap) return 0;
    return src->len;
}

bool buffer_push_bytes(Buffer *dest, const unsigned char *src, size_t count)
{
    assert(NULL != dest);
    assert(NULL != src);

    for(size_t i = 0; i < count; ++i) {
        if(!buffer_push_byte(dest, src[i]))
        {
            log_message("Unable to push byte %zu", i);
            return false;
        }
    }

    return true;
}

void buffer_assign_recycler(Buffer *buf, Recycler *rc) {
    assert(buf != NULL);
    buf->recycler = rc;
}

bool buffer_is_null_terminated(const Buffer *buf) {
    assert(NULL != buf);
    return (buf->nullTerminated);
}

bool buffer_is_ascii(const Buffer *buf) {
    assert(NULL != buf);

    if(buffer_is_empty(buf)) return true;

    for(size_t i=0; i < buf->len; ++i) {
        if(isascii(buf->data[i])) continue;
        if(0 == buf->data[i] && buf->nullTerminated == true) return true;
        return false;
    }

    return true;
}

void buffer_dump(const Buffer *buf) {
    assert(NULL != buf);
    bool ascii = true;
    fprintf(stderr, "\t\tBuffer - len[%zu] cap[%zu] recycler[%p] nullterminated[%i] value",
            buf->len, buf->cap, buf->recycler,  buf->nullTerminated);
    fprintf(stderr, "[");
    if(NULL == buf->data) {
        fprintf(stderr, "NULL");
    }
    else {
        if(buffer_is_ascii(buf)) {
            if(buf->nullTerminated) {
                fprintf(stderr, "%s", (char *) buf->data);
            }
            for(size_t i=0; i<buf->len; ++i) {
                if(buf->data[i] == 0) break;
                fputc(buf->data[i], stderr);
            }
        } else {
            fprintf(stderr, "<BINARY>");
        }
    }

    fprintf(stderr, "]\n");
}

char * buffer_get_string(Buffer *src) {
    assert(NULL != src);

    if(buffer_make_string(src)) return (char *) src->data;
    return NULL;
}

char * buffer_memchr(Buffer *src, char c) {
    assert(NULL != src);

    char * ret = (char *) src->data;
    const char * end = (char *) src->data + src->len;

    if(NULL == ret) return ret;

    return memchr(src->data, c, src->len);
}

char * buffer_get_data(Buffer * src) {
    assert(NULL != src);
    return (char *) src->data;
}