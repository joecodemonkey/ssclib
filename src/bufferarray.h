//
// Created by Joseph Hurdle on 7/14/20.
//

#ifndef SEARCHFILEC_BUFFERARRAY_H
#define SEARCHFILEC_BUFFERARRAY_H

#include "buffer.h"
#include "recycler.h"

typedef struct stBufferArray {
    Buffer array;
    size_t count;
    Recycler * recycler;
} BufferArray;

/* intialize a bufferarray [ba] with sane defaults */
void buffer_array_init(BufferArray *ba);

/* push a copy of buffer [buf] onto buffer array [ba]
 * [ba] - ba to get new buffer
 * [buf] - buf to push onto array
 * returns true on success, fails on memory issues
 * */
bool buffer_array_push(BufferArray *ba, const Buffer *buf);

/* copy data stored within a bufferarray [ba] at index [idx] to an
 * output buffer [out] use buffer_array_get_buffer to get a pointer
 * to the internally held buffer
 * [ba] - bufferarray to copy data from
 * [idx] - index of buffer in buffer array to copy
 * [out] - pointer to out buffer where data will land
 * returns true on success, false on failure (insufficient memory)
 * */
bool buffer_array_cpy_buffer(BufferArray *ba, size_t idx, Buffer *out);

/* get a pointer to buffer held in bufferarray [ba] at array index [idx]
 * [ba] - buffer array to get pointer from
 * [idx] - index of buffer to get pointer to
 * returns a valid buffer pointer on success or a NULL if idx is out of bounds
*/
Buffer * buffer_array_get_buffer(BufferArray *ba, size_t idx);

/* remove the buffer at index [idx] from bufferarray [ba]
 * [ba] - buffer array to remove buffer from
 * [idx] - index of buffer to remove
 */
void buffer_array_remove_buffer(BufferArray *ba, size_t idx);

/* get the count of the buffers held within a bufferarray [ba]
 * [ba] - buffer array to get count from
 * returns count of buffers in array
 */
size_t buffer_array_get_buffer_count(BufferArray *ba);

/* free a bufferarray [ba] and all buffers contained within it
 * [ba] - bufferarray to free
 * */
void buffer_array_free(BufferArray *ba);

/* get the capacity of a bufferarray [ba], that is the number
 * of buffers which may be held before the array will have to alloacte
 * more memory, this includes currently active buffers
 * [ba] - bufferarray to get capacity from
 * returns total capacity of buffer array
 */
size_t buffer_array_get_capacity(BufferArray *ba);

/* assign recylcer [r] to bufferarray [ba] so that memory may be recyled intead
 * of being returned using free
 * [ba] - bufferarray to assign recycler to
 * [r] - recycler to be assigned
 */
void buffer_array_assign_recycler(BufferArray *ba, Recycler *rc);

/* modify a buffer array [dest] to point at the internal data structures of
 * another [src] note that if both buffer arrays continue to be used, they
 * will diverge when memory is eventually allocated
 * [dest] - buffer array to clone into
 * [src] - buffer array to be cloned
 * */

void buffer_array_clone(BufferArray *dest, BufferArray *src);

#endif //SEARCHFILEC_BUFFERARRAY_H
