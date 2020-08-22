//
// Created by Joseph Hurdle on 7/5/20.
//

#ifndef SEARCHFILEC_HASHTABLE_H
#define SEARCHFILEC_HASHTABLE_H

#include "buffer.h"
#include <stdbool.h>
#include "recycler.h"
#include "bufferarray.h"

#define HASH_TABLE_DEFAULT_SIZE 10

typedef Buffer HashKey;


/* HashValue
 * A hashvalue is a HashKey (buffer of data) and
 * a data element which may be stored or retrieved from a hash table.
 */

typedef struct stHashedValue {
    HashKey key;
    Buffer data;
    Recycler *recycler;
} HashValue;

/* HashTuple
 * A hashtuple is a collection of hashvalues which wind up at a particualar
 * hash table index.  An array is used to manage collisions
 * a data element which may be stored or retrieved from a hash table.
 */

typedef struct stHashTuple {
    BufferArray buffer;
    Recycler *recycler;
} HashTuple;

 /* HashTable
 * A hash table is a data structure which allows ~O(1) access to its members
 * The hashtable uses a hash key which is just a buffer containing bytes
 * and stores any data in another buffer containing bytes
 */

typedef struct stHashTable {
    BufferArray table;
    size_t valueCount;
    size_t size;
    Recycler *recycler;
} HashTable;

/* initialize a hash value [hv] so that it is ready to be populated
   [hv] - hash value to be initialized
*/
void hashvalue_init(HashValue *hv);

/* retrieve the HashKey portion of a hash value [hv]
   [hv] - the hash value to retrieve the key from
   returns a HashKey pointer
*/
HashKey * hashvalue_getkey(HashValue *hv);

// retrieve the value stored in a hash value [hv] datastructure
// [hv] - the hash value to retrieve the data from
// returns a HashKey pointer
Buffer * hashvalue_getdata(HashValue *hv);

// free any internal memory used by a hash value [hv] it is the responsibility
// of the caller to free any memory allocated for the hash value itself
// [hv] - the hash value to free
void hashvalue_free(HashValue *hv);

// assign a recycler [r] to a hashvalue [v] data structure so memory may be
// recycled instead of being freed useing free//
// [hv] - the hash value to free
// [r] - recycler to assign
void hashvalue_assign_recylcer(HashValue *hv, Recycler *r);

/* copy a hashvalue [src] to a hashvalue [dest], allocating memory as needed
 * [src] - source hashvalue to copy from
 * [dest] - dest hashvalue to copy to
 * returns true on success, fails on memory availability issues
*/
bool hashvalue_cpy(HashValue *dest, const HashValue * src);

/* initialize a hashtable [ht] so that it is ready to use
   [ht] - the hash table to be initialized
*/
void hashtable_init(HashTable *ht);

/* Add a value [value] to a hash table [ht] so it may be looked up using a
 * key [key]
 * [ht] - hash table to add value to
 * [key] - key used to retrieve value
 * [value] - value to be retrieved
 returns true on success, will only fail on memory allocation failure
*/
bool hashtable_add(HashTable *ht, const HashKey *key, const Buffer *value);

/* Remove a key [hk] and any data stored using that key to hash table [ht]
 * [ht] - hash table to remove key from
 * [hk] - key to remove
*/
void hashtable_remove(HashTable *ht, const HashKey *hk);

/* Retrieve a pointer to internal buffer holding stored in hashtable [ht]
 * using key [key]
 * [ht] - hash table from which to retrieve key
 * [key] - key of data to retrieve
 * returns pointer to buffer containing data or NULL if data does not exist
 * within hash table.  Freeing this buffer will result in undefined behaviour
 */
Buffer *hashtable_get(HashTable *ht, const HashKey *key);

/* Check hashtable [ht] to see if it has any data stored using key [key]
 * [ht] - hash table to check
 * [key] - key to check hash table for
 * returns bool if key exists in hash table, false if it does not
*/

bool hashtable_has(const HashTable *ht, const HashKey *key);

/* assign recylcer [r] to hashtable [ht] so that memory may be recyled intead
 * of being returned using free
 * [ht] - hashtable to assign recycler to
 * [r] - recycler to be assigned
 */
void hashtable_assign_recycler(HashTable *ht, Recycler *r);

/* free any data held within a hashtable [ht] or its children */
void hashtable_free(HashTable *ht);

/* set the size of the hashtable [ht] to [size] entries and rearange data
 * within it accordingly.  This operation will be costly both in terms of
 * memory and time for hash tables which have already been populated
 * [ht] - hash table to be resized
 * [size] - new size for hash table
 * returns true on success, false on failure which is likely a memory limitation
 */
bool hashtable_set_size(HashTable *ht, size_t size);

/* get the size of the hashtable [ht].  size here is not the same as the
 * number of pieces of data actually stored in the hash table.  for that
 * use hashtable_get_entry_count
 * [ht] - hashtable to get count for
 * returns size of the hash table
 */
size_t hashtable_get_size(const HashTable *ht);

/* get the number of entries in hashtable [ht].  the number of entries is the
 * number of unique pieces of data stored, not the size of the hash table
 * for that use hashtable_get_size
 * [ht] - hashtable to return size of
 * returns size of hash table
 */
size_t hashtable_get_entry_count(const HashTable *ht);

/* point one hash table's [dest] internal data structures to another's [src]
 * [dest] - hash table to clone into
 * [src] - hash table to clone from
 *
 * */

void hashtable_clone(HashTable *dest, HashTable *src);

void hashvalue_dump(HashValue *src);
void hashtuple_dump(HashTuple *src);
void hashtable_dump(HashTable *src);

HashTuple *hashtable_get_hastuple_at_idx(HashTable *ht, size_t idx);


/* assign a recycler [r] to a hashtuple [ht] data structure so memory may be
   recycled instead of being freed using free
   [ht] - the hashtuple to free
   [r] - recycler to assign
*/
void hashtuple_assign_recylcer(HashTuple *ht, Recycler *r);

/* free a hashtuple's [ht] internal data structures, it is up to the
 * caller to free the hashtuple itself
 * [ht] - hashtuple to free
*/
void hashtuple_free(HashTuple *ht);

/* remove the key [key] and its associated data from a hash tuple [ht]
 * [ht] - hash tuple to remove key from
 * [key] - key to remove
*/
void hashtuple_remove(HashTuple *ht, const HashKey *key);

/* return the associated data buffer from the hash tuple [ht] using hash key [hk]
 * [ht] - hash tuple to search
 * [key] - key to search for
 * returns the buffer pointer for the data if found or, returns null
*/
Buffer * hashtuple_get(HashTuple * ht, const HashKey *key);

/* return the count of items contained in hashtuple [ht]
 * ht - hash tuple to get count from
 * returns count
 */
size_t hashtuple_get_count(HashTuple *ht);

/* search a hash tuple [ht] for the hash key [hk] and populate its index to
 * the pointer [indexOut]
 * [ht] - hash tuple to search
 * [hk] - key to search for
 * [indexOut] - a pointer to a size_t where the index should be written when
 * found
 * returns true if key is found
*/
bool hashtuple_find_key_index(HashTuple * ht, const HashKey *key,
                              size_t *indexOut);

/* return the hash value stored at index [idx] in hashtuple [ht]
 * [ht] - hash tuple to return value from
 * [idx] - index of hash value to return
 * returns a pointer to the hash value or, NULL if the index is out
 * of bounds
*/
HashValue * hashtuple_get_hash_value_at_idx(HashTuple * ht, size_t idx);

// add a hashvalue [hv] to a HashTyple [ht]
// [ht] - hash tuple to add value to
// [hv] - hash value to add to the tuple
// returns true on success, only fails due to memory constraints
bool hashtuple_add(HashTuple *ht, const HashValue *hv);

// initialize a hashtuple [ht] with sane defaults
// [ht] - the hash tuple to be initialized
void hashtuple_init(HashTuple *ht);

HashValue * hashtuple_get_hash_value_at_idx(HashTuple * ht, size_t idx);

#endif