//
// Created by Joseph Hurdle on 7/4/20.
//

#ifndef SEARCHFILEC_BUFFER_H
#define SEARCHFILEC_BUFFER_H

#define CAPACITY_INCREMENT 10

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "log.h"
#include "recycler.h"

typedef struct stBuffer {
    unsigned char *data;
    size_t len;
    size_t cap;
    bool nullTerminated;
    Recycler *recycler;
} Buffer;

#include "bufferarray.h"

/*
 * the Buffer data structure is a general purpose dynamic array of bytes
 * useful for storing pretty much anything and making it simpler to work with
*/

// initialize a buffer [data] with sane defaults
// [buf] - buffer to be initialized
void buffer_init(Buffer *buf);

// compare to buffers [a] and [b], used for sorting and binary search
// note that my implemention is't precisely alphabetic order as I use
// length as a shortcut against an actual bytewise comparison
// [a] - buffer a
// [b] - buffer b
// returns a <=> b
int buffer_cmp(const void *a, const void*b);

// swap the internals buffer [a] and buffer [b]
// [a] - first buffer
// [b] - second buffer
void buffer_swap(Buffer *a, Buffer *b);

// reserve [bytes] bytes in a buffer [buf], adjusting capacity
// any existing data is in the buffer, it is preserved
// [buf] - buffer to get new capacity
// [bytes] - number of bytes to reserve
// returns true if data successfully expanded
bool buffer_reserve(Buffer *buf, unsigned int bytes);

// free internal memory held by buffer [data]
// [buf] - buffer which owns internal meomry to be freed
// if a recycler is attached, the memory is recycled
// after freeing memory owned by data, it's the responsibility of
// the caller to free the memory of data itself
void buffer_free(Buffer *buf);

// quickly check and see if buffer [buf] is empty
// [buf] - buffer to check
// returns true if the buffer is empty, false if not
bool buffer_is_empty(const Buffer *buf);

// quickly check and see if buffer [buf] is null terminated
// [buf] - buffer to check
// returns true if the buffer is empty, false if not
bool buffer_is_null_terminated(const Buffer *buf);

// check and see how much free space is left in a buffer [data]
// [data] - buffer to check
// returns buffers capacity less the number of bytes it currently holds
size_t buffer_get_freespace(const Buffer *buf);

// copy buffer [src] data to [dest], increasing capacity of [dest] if needed
// and overwriting the contents of src
// [dest] - buffer to get new data
// [src] - buffer whose data will be copied
// returns true if the copy worked, false if not
bool buffer_cpy(Buffer *dest, const Buffer *src);

// clone buffer [src] data to [dest], so that the two buffers point at the same
// internal buffer of memory (use this with caution)
// [dest] - buffer to clone into
// [src] - buffer to clone from
// returns true if the copy worked, false if not
void buffer_clone(Buffer *dest, const Buffer *src);

// append buffer [src] to buffer [dest], increasing capacity of [dest] if needed
// [dest] - buffer to get new data
// [src] - buffer whose data will be appended to dest
// returns true if the append worked, false if not
bool buffer_append(Buffer *dest, const Buffer *src);

// push a single byte onto the back of the buffer, adding memory if needed
// [dest] - buffer onto which byte will be pushed
// [c] - byte to be pused
// returns true on success, false on failure
bool buffer_push_byte(Buffer *dest, unsigned char c);

// reset the buffer [buf] so that any data in it is gone
// without freeing any internal memory
// [buf] - buffer to be reset
void buffer_clear(Buffer *buf);

// copy a null terminated string [str] into a buffer [buf]
// overwrites conents of buffer on copy
// this action does not make the buffer null terminated
// [buf] - buffer to copy into
// [str] - null terminated string to copy
// returns true on success, false on failure
bool buffer_strcpy(Buffer *buf, const char *str);

// ensure a buffer [buf] is null terminated, allocating more spaced if needed
// after this action, the buffer will forever remain null terminated
// no matter what data is added, but does not gaurantee buffer is actually
// safe to use for string functions
// [data] - buffer to place the null on
// return - true if null was able to be placed on end of buffer
bool buffer_make_string(Buffer *buf);

// shift the contents of the buffer [buf] to the left, adjusting length
// [buf] - buffer to be shifted
// [off] - amount to shift buffer
// returns true on success
bool buffer_lshift(Buffer *buf, size_t off);

// trim a string  [suffix] from the end of a buffer [buf]
// [buf] - buffer to be trimmed
// [suffix] - suffix to trim
void buffer_trim_suffix(Buffer *buf, const char *suffix);

// lowercase all alpha characters in a buffer [buf]
// [buf] - buffer to be downcased
void buffer_downcase(Buffer *line);

// clean any nonalphanumeric text from a buffer [buf], leaving (single) spaces
// converts double spaces to single, only spaces and alpha characters are
// preserved
// [buf] - buffer to be cleansed
// returns true if successful
void buffer_cleanse_text(Buffer *line);

// split a buffer [src] using a single character delimter [delim] writing the
// tokens to a buffer array [out]
// [src] - buffer to be split
// [delim] - delimiter to split using
// [out] - buffer array to write results to
// returns true on success
bool buffer_split(const Buffer *src, unsigned char delim, BufferArray *out);

// return the total capacity of a buffer [src]
// [src] - buffer to determine capacity of
// returns capacity
size_t buffer_get_capacity(const Buffer *src);

// return the total size/length of the data in a buffer [src] irrespective
// of the buffer's capacity
// [src] - buffer to determine size of
// returns size/length
size_t buffer_get_size(const Buffer *src);

// push a string of bytes from [src] of length [count] onto buffer [dest]
// allocating memory as needed
// [dest] - destination to recieve bytes
// [src] - source array containing bytes to be pused
// [count] - number of bytes to push
// returns true on success
bool buffer_push_bytes(Buffer *dest, const unsigned char *src, size_t count);

// Assign a recylcer [rc] to a buffer [buf] so that memory may be recycled
// instead of being instantly being freed, preventing calls to malloc/free
// [buf] - buffer which will now have use of recycler
// [rc] - recycler buffer will have use of
void buffer_assign_recycler(Buffer *buf, Recycler *rc);

bool buffer_isascii(const Buffer *buf);
void buffer_dump(const Buffer *buf);

char * buffer_get_string(Buffer *src);
char * buffer_get_data(Buffer * src);

char * buffer_memchr(Buffer *src, char c);


#endif //SEARCHFILEC_BUFFER_H
