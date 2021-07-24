//
// Created by Joseph Hurdle on 7/5/20.
//

#include <assert.h>
#include "hashtable.h"
#include "recycler.h"
#include <string.h>
#include <stdio.h>
#include "filereader.h"
//#include <math.h>
#include "log.h"

void hashvalue_init(HashValue *hv) {
    assert(NULL != hv);
    hv->recycler = NULL;
    buffer_init(&hv->data);
    buffer_init(&hv->key);
}

HashKey * hashvalue_getkey(HashValue *hv) {
    assert(NULL != hv);
    return &hv->key;
}

Buffer * hashvalue_getdata(HashValue *hv) {
    assert(NULL != hv);
    return &hv->key;
}
void hashvalue_free(HashValue *hv) {
    assert(NULL !=hv);
    buffer_free(&hv->data);
    buffer_free(&hv->key);
}

void hashvalue_assign_recylcer(HashValue *hv, Recycler *r) {
    assert(NULL != hv);
    hv->recycler = r;
    buffer_assign_recycler(&hv->key, r);
    buffer_assign_recycler(&hv->data, r);
}

size_t tenpow(size_t exponent) {
    size_t ret = 1;
    for(size_t i=0; i<exponent; ++i) {
        ret *= 10;
    }
    return ret;
}

size_t maxdigits() {
    size_t x = (size_t) -1; // should overflow to max size_t
    size_t count = 0;
    while(x > 0) {
        ++count;
        x /= 10;
    }
    return count;
}

// compute the hash of any data stored in a hashkey [hk] will return 0 if
// no data is stored within the hashkey
// [hk] - the hash key to compute the hash for
size_t hashkey_compute_hash(const HashKey *hk) {
    assert(NULL != hk);

    size_t ret = 0;

    if(buffer_is_empty(hk)) return ret;

    const size_t len = buffer_get_size(hk);
    size_t maxDigits = maxdigits() - 2;
    while(maxDigits % 3 != 0) {
        --maxDigits;
    }

    ///   HELLO
    ///   ASCII H (53)
    ///   ASCII E 22 -> 22000 + 53 = 022053
    for(size_t i=0; i<len; ++i) {
        if(i< maxDigits)
            ret += hk->data[i] * tenpow(i);
        else {
            size_t tmp = hk->data[i] * tenpow(i);
            ret ^= tmp;
        }
    }
    return ret;
}


// compute the hash of any the key stored in a hashvalue [hv] will return 0 if
// the hashvalue's hashkey has no data
// [hv] - the hash value whose key to compute the hash for
size_t hashvalue_compute_hash(const HashValue *hv) {
    assert(NULL != hv);
    return hashkey_compute_hash(&hv->key);
}


bool hashvalue_cpy(HashValue *dest, const HashValue * src) {
    assert(NULL != dest);
    assert(NULL != src);

    // allocate a tmp buffer to hold what was in dest data so we
    // can recover it in the event that it copies and the key does not
    Buffer tmpData;
    buffer_init(&tmpData);
    buffer_clone(&tmpData, &dest->data);

    buffer_init(&dest->data);
    dest->recycler = tmpData.recycler;
    
    if(!buffer_cpy(&dest->data, &src->data)) {
        log_message("unable to copy data buffer");
        return false;
    }

    if(!buffer_cpy(&dest->key, &src->key)) {
        log_message("unable to copy key buffer");
        buffer_clone(&dest->data, &tmpData);
        dest->recycler = tmpData.recycler;
        return false;
    }

    buffer_free(&tmpData);
    return true;
}

/* compute hash/index for hashkey [hk] and hashtable [ht]
 * this computes the hash then takes into account the size of the hash
 * table
 * [ht] - hashtable to compute hash using
 * [hk] - key whose hash should be computed
 * returns a numeric hash or 0 if the hash table is of size 0
 */
size_t hashtable_compute_hash(const HashTable *ht, const HashKey *hk) {

    assert(NULL != ht);
    assert(NULL != hk);

    if(0 == ht->size) return 0;

    return hashkey_compute_hash(hk) % ht->size;
}

void hashtuple_init(HashTuple *ht) {
    assert(NULL != ht);
    ht->recycler = NULL;
    buffer_array_init(&ht->buffer);
}

bool hashtuple_add(HashTuple *ht, const HashValue *hv) {
    assert(NULL != ht);
    assert(NULL != hv);


    HashValue * newHV = NULL;
    const size_t size = sizeof(HashValue);

    if(ht->recycler) {
        newHV = recycler_get_exact(ht->recycler, size);
    }
    else {
        newHV = malloc(size);
    }

    if(NULL == newHV) {
        log_message("unable to allocate memory to hold a hashvalue");
        return false;
    }

    hashvalue_init(newHV);
    hashvalue_assign_recylcer(newHV, ht->recycler);

    if(!hashvalue_cpy(newHV, hv)) {
        log_message("unable to copy hashvalue into hashvalue in buffer");
        return false;
    }

    Buffer tmp;
    buffer_init(&tmp);
    buffer_assign_recycler(&tmp, ht->recycler);
    if(!buffer_push_bytes(&tmp, (unsigned char *) newHV, sizeof(HashValue))) {
        log_message("Unable to push hashvalue into buffer.");
        return false;
    }



    buffer_assign_recycler(&tmp, ht->recycler);
    if(!buffer_array_push(&ht->buffer, &tmp)) {
        log_message("Failure adding hash value to internal hash tuple array");
        buffer_free(&tmp);
        return false;
    }

    buffer_free(&tmp);

    return true;
}

HashValue * hashtuple_get_hash_value_at_idx(HashTuple * ht, size_t idx) {
    assert(NULL != ht);

    if(buffer_array_get_buffer_count(&ht->buffer) <= idx) return NULL;

    Buffer *b = buffer_array_get_buffer(&ht->buffer, idx);

    if(NULL == b) return NULL;

    return (HashValue *) b->data;
}


bool hashtuple_find_key_index(HashTuple * ht, const HashKey *key,
        size_t *indexOut) {
    assert(NULL != ht);
    assert(NULL != key);

    if(NULL == key->data) return false;

    HashValue *hv = NULL;
    Buffer buf;

    const size_t len = buffer_array_get_buffer_count(&ht->buffer);
    for(size_t i=0; i<len; ++i)  {
        hv = hashtuple_get_hash_value_at_idx(ht, i);
        if(NULL == hv) continue;
        if(NULL == hv->key.data) continue;
        if(hv->key.len != key->len) continue;
        if(memcmp(hv->key.data, key->data, key->len) != 0) continue;

        *indexOut = i;
        return true;
    }

    return false;
}

size_t hashtuple_get_count(HashTuple *ht) {
    assert(NULL != ht);
    return buffer_array_get_buffer_count(&ht->buffer);
}


Buffer * hashtuple_get(HashTuple * ht, const HashKey *key) {
    assert(NULL != ht);
    assert(NULL != key);

    if(NULL == key->data) return NULL;

    size_t idx = 0;

    if(hashtuple_find_key_index(ht, key, &idx)) {
        HashValue * hv = hashtuple_get_hash_value_at_idx(ht, idx);
        if(NULL != hv) return hashvalue_getdata(hv);
    }

    return NULL;
}

void hashtuple_remove(HashTuple *ht, const HashKey *key) {
    assert(NULL != ht);
    assert(NULL != key);

    if(NULL == key->data) return;

    size_t idx = 0;

    if(hashtuple_find_key_index(ht, key, &idx)) {
        buffer_array_remove_buffer(&ht->buffer, idx);
    }
}

void hashtuple_free(HashTuple *ht) {
    assert(NULL != ht);
    buffer_array_free(&ht->buffer);
    buffer_array_assign_recycler(&ht->buffer, ht->recycler);
}


void hashtuple_assign_recylcer(HashTuple *ht, Recycler *r) {
    assert(NULL != ht);
    buffer_array_assign_recycler(&ht->buffer, r);

    const size_t len = buffer_array_get_buffer_count(&ht->buffer);
    HashValue  * hv = NULL;
    for(size_t i = 0; i < len; ++i) {
        hv = hashtuple_get_hash_value_at_idx(ht, i);
        if(NULL != hv) hashvalue_assign_recylcer(hv, r);
    }
}


void hashtable_init(HashTable *ht) {
    assert(NULL != ht);

    ht->recycler = NULL;
    buffer_array_init(&ht->table);
    ht->size = HASH_TABLE_DEFAULT_SIZE;
    ht->valueCount = 0;
}

HashTuple *hashtable_get_hastuple_at_idx(HashTable *ht, size_t idx) {
    assert(NULL != ht);
    if(0 == ht->size) return NULL;

    Buffer * b = buffer_array_get_buffer(&ht->table, idx);

    if(NULL == b) {
        return NULL;
    }

    HashTuple  * ret;

    if(buffer_is_empty(b)) {
        if(buffer_reserve(b, sizeof(HashTuple))) {
            ret = (HashTuple *) b->data;
            hashtuple_init(ret);
        }
        else {
            log_message("unable to reserve memory for a hashtuple");
        }
    }

    return (HashTuple *) b->data;
}

HashTuple *hashtable_get_hashtuple(HashTable *ht, const HashKey *hk) {
    assert(NULL != ht);
    assert(NULL != hk);

    const size_t hsh = hashtable_compute_hash(ht, hk);
    HashTuple *hashTuple = hashtable_get_hastuple_at_idx(ht, hsh);

    if(NULL == hashTuple) {
        log_message("Error: unable to retrieve hashtuple");
        return NULL;
    }

    return hashTuple;
}

bool hashtable_add(HashTable *ht, const HashKey *key, const Buffer *value) {
    assert(NULL != ht);
    assert(NULL != value);
    assert(NULL != key);

    HashValue hv;

    // this only happens if the hash table is empty
    if(ht->table.count < ht->size) {
        if(!hashtable_set_size(ht, ht->size)) {
            log_message("unable to add an item as hash table cannot be expanded");
            return false;
        }
    }

    hashvalue_init(&hv);
    buffer_clone(&hv.data, value);
    buffer_clone(&hv.key, key);

    bool newKey = false;
    HashTuple * hashTuple = hashtable_get_hashtuple(ht, key);
    if(hashtuple_get(hashTuple, key) == NULL) newKey = true;

    if(NULL == hashTuple) {
        log_message("Error, hashtable_add returned null tuple?");
        return false;
    }

    if(!hashtuple_add(hashTuple, &hv)) {
        log_message("Error, unable to add hashvalue to hashuple");
        return false;
    }
    if(newKey) ht->valueCount++;
    return true;

}

Buffer *hashtable_get(HashTable *ht, const HashKey *hk) {
    assert(NULL != ht);
    assert(NULL != hk);

    HashTuple * tuple = hashtable_get_hashtuple(ht, hk);

    if(NULL == tuple) {
        log_message("Error: unable to retrieve hashtuple");
        return NULL;
    }
    return hashtuple_get(tuple, hk);
}

void hashtable_remove(HashTable *ht, const HashKey *hk) {

    assert(NULL != ht);
    assert(NULL != hk);

    HashTuple * tuple = hashtable_get_hashtuple(ht, hk);
    if(NULL == tuple) {
        log_message("Error: unable to retrieve hashtuple");
        return;
    }

    hashtuple_remove(tuple, hk);
}

void hashtable_assign_recycler(HashTable *ht, Recycler *r) {
    assert(NULL != ht);
    ht->recycler = r;
    if(ht->table.count == 0) return;

    ht->table.recycler = r;
    for(size_t i = 0; i < ht->size; ++i) {
        HashTuple * tuple = hashtable_get_hastuple_at_idx(ht, i);
        if(NULL != tuple) {
            continue;
        }
        hashtuple_assign_recylcer(tuple, r);
    }
}

void hashtable_free(HashTable *ht) {
    assert(NULL != ht);

    const size_t count = buffer_array_get_buffer_count(&ht->table);

    for(size_t i=0; i<count; ++i) {
        HashTuple * tuple = hashtable_get_hastuple_at_idx(ht, i);
        if(NULL != tuple) hashtuple_free(tuple);
    }
}
size_t hashtable_get_size(const HashTable *ht) {
    assert(NULL != ht);
    return ht->size;
}

size_t hashtable_get_entry_count(const HashTable *ht) {
    assert(NULL != ht);
    return ht->valueCount;
}

void hashtable_clone(HashTable *dest, HashTable *src) {

    assert(NULL != dest);
    assert(NULL != src);

    dest->size = src->size;
    dest->recycler = src->recycler;
    dest->valueCount = src->valueCount;
    buffer_array_clone(&dest->table, &src->table);
}

bool hashtable_set_size(HashTable *ht, size_t size) {

    assert(NULL != ht);

    if(size == 0) {
        log_message("Attempt to set hash table size to 0");
        return false;
    }

    if(ht->size == size) {
        if(buffer_array_get_buffer_count(&ht->table) == size) return true;
    }

    HashTable htNew;
    hashtable_init(&htNew);
    htNew.size = size;
    htNew.recycler = ht->recycler;

    const size_t oldHTSize = ht->size;

    Buffer bufferTmp;
    buffer_init(&bufferTmp);
    buffer_assign_recycler(&bufferTmp, ht->recycler);

    HashTuple hashTupleTmp;
    hashtuple_init(&hashTupleTmp);

    hashtuple_assign_recylcer(&hashTupleTmp, ht->recycler);

    if(!buffer_push_bytes(&bufferTmp, (unsigned char *) &hashTupleTmp, sizeof(HashTuple))) {
        log_message("Unable to push hash tuple onto buffer");
        return false;
    }

    for(size_t i=0; i < ht->size; ++i) {
        if(!buffer_array_push(&htNew.table, &bufferTmp)) {
            log_message("Unable to push buffer onto hash table");
            buffer_free(&bufferTmp);
            hashtable_free(&htNew);
            return false;
        }
    }

    buffer_free(&bufferTmp);

    for (size_t i = 0; i < oldHTSize ; ++i) {

        HashTuple * hashTuple = hashtable_get_hastuple_at_idx(ht, i);

        if(NULL == hashTuple) {
           continue;
        }

        const size_t hashTupleCount = hashtuple_get_count(hashTuple);

        for(size_t j=0; j < hashTupleCount; ++j) {

            HashValue * hv = hashtuple_get_hash_value_at_idx(hashTuple, j);

            if(NULL == hv) continue;

            if(!hashtable_add(&htNew, &hv->key, &hv->data)) {

                log_message("Unable to add hashvalue to new hash table");
                hashtable_free(&htNew);

                return false;
            }
        }
    }

    hashtable_free(ht);
    hashtable_clone(ht, &htNew);
    return true;
}


void hashvalue_dump(HashValue *src) {
    assert(NULL != src);

    fprintf(stderr, "\t\tHashValue -\n");

    fprintf(stderr, "\t\t\tKey - ");
    buffer_dump(&src->key);

    fprintf(stderr, "\t\t\tValue - ");
    buffer_dump(&src->data);
}

void hashtuple_dump(HashTuple *src) {
    assert(NULL != src);
    fprintf(stderr, "\t\tHashTuple: Count: %zu\t ", src->buffer.count);
    for(size_t i=0; i< src->buffer.count; ++i) {
        Buffer *buf = buffer_array_get_buffer(&src->buffer, i);
        if (NULL == buf) {
            fprintf(stderr, " <NULL HASHTUPLE>");
        } else if (NULL == buf->data) {
            fprintf(stderr, " <NULL HASHVALUE>");
        } else {
            HashValue *hv = (HashValue *) buf->data;
            hashvalue_dump(hv);
        }
    }
}


void hashtable_dump(HashTable *src) {
    assert(NULL != src);

    fprintf(stderr, "+");
    for(size_t i=0; i<75; ++i) fprintf(stderr, "-");
    fprintf(stderr, "+\n");


    fprintf(stderr, "\tHash Table\n");
    fprintf(stderr, "\t\tSize: %zu\n", src->size);
    fprintf(stderr, "\t\tValue Count: %zu\n\n", src->valueCount);

    for(size_t i=0; i<src->size; ++i) {

        HashTuple *ht = hashtable_get_hastuple_at_idx(src, i);
        fprintf(stderr, "\t\tHash: %zu", i);
        if(NULL == ht) {
            fprintf(stderr, "<NULL HASH TUPLE>");
        } else {
            hashtuple_dump(ht);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "+");
    for(size_t i=0; i<75; ++i) fprintf(stderr, "-");
    fprintf(stderr, "+\n");

}
