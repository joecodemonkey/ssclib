//
// Created by Joseph Hurdle on 7/24/20.
//

/* file minunit_example.c */

#include <stdio.h>
#include "simpletest.h"
#include "../src/buffer.h"
#include "../src/bufferarray.h"
#include "../src/recycler.h"
#include "../src/bufferarray.h"
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "../src/hashtable.h"

void buffer_init_test() {
    Buffer b;
    buffer_init(&b);

    simple_test_assert("Buffer Capacity does not init to 0",
            buffer_get_capacity(&b) == 0);

    simple_test_assert("Buffer does not init to empty",
                       buffer_is_empty(&b));

    simple_test_assert("Buffer size != 0 after init ",
            buffer_get_size(&b) == 0);

    simple_test_assert("Buffer freespace does not init to 0",
            buffer_get_freespace(&b) == 0);
}

void buffer_reserve_test(Recycler * recycler) {
    Buffer b;
    buffer_init(&b);
    buffer_assign_recycler(&b, recycler);

    simple_test_assert("Buffer Reserve Fails",
                       buffer_reserve(&b, 50));

    simple_test_assert("Buffer Capacity is not set by reserve",
                       buffer_get_capacity(&b) == 50);

    simple_test_assert("Empty Buffer not empty after reserve",
                       buffer_is_empty(&b));

    simple_test_assert("Buffer freespace not correct after reserve",
                       buffer_get_freespace(&b) == 50);

    buffer_free(&b);

    simple_test_assert("Buffer Capacity is not set by free",
                       buffer_get_capacity(&b) == 0);

    simple_test_assert("Empty Buffer not empty after free",
                       buffer_is_empty(&b));

    simple_test_assert("Buffer freespace not 0 after free",
                       buffer_get_freespace(&b) == 0);

}

void buffer_set_test(Recycler * recycler) {
    Buffer b;
    buffer_init(&b);
    buffer_assign_recycler(&b, recycler);
    buffer_push_byte(&b, 'c');
    buffer_make_string(&b);
    simple_test_assert("Buffer Pushbyte Fails",
            (strcmp("c", (char *) b.data) == 0));

    buffer_free(&b);
    buffer_strcpy(&b, "Taco");
    simple_test_assert("Buffer buffer_strcmp Fails",
                       (strcmp("Taco", (char *) b.data) == 0));

    Buffer c;
    buffer_init(&c);
    buffer_assign_recycler(&b, recycler);
    buffer_cpy(&b, &c);

    simple_test_assert("Buffer buffer_cpy Fails",
                       (strcmp("Taco", (char *) b.data) == 0));

    buffer_free(&c);
    buffer_free(&b);
}

void buffer_transform_test(Recycler * recycler) {
    Buffer b;
    buffer_init(&b);
    buffer_assign_recycler(&b, recycler);
    buffer_make_string(&b);
    buffer_strcpy(&b, "Taco");

    buffer_downcase(&b);
    simple_test_assert("Buffer downcase fails",
                       strcmp((char *) b.data, "taco") == 0);

    buffer_lshift(&b, 3);
    simple_test_assert("leftshift fails",
                       strcmp((char *)b.data, "o") == 0);
    buffer_free(&b);
}

void buffer_array_test(Recycler * recycler) {
    Buffer b;
    buffer_init(&b);
    BufferArray ba;
    buffer_array_init(&ba);
    buffer_assign_recycler(&b, recycler);
    buffer_strcpy(&b, "Hello From Mars");

    simple_test_assert("Buffer array retrieves buffer 0 when empty",
                       !buffer_array_cpy_buffer(&ba, 0, &b));

    simple_test_assert("Buffer array retrieves buffer 1 when empty",
                       !buffer_array_cpy_buffer(&ba, 1, &b));

    simple_test_assert("Buffer array fails to add buffer",
                       buffer_array_push(&ba, &b));
}


void buffer_split_test(Recycler * recycler) {

    Buffer b;
    buffer_init(&b);
    BufferArray ba;
    buffer_array_init(&ba);
    buffer_assign_recycler(&b, recycler);
    simple_test_assert("Buffer strcpy fails.",
                       buffer_strcpy(&b, "Hello From Mars"));

    buffer_make_string(&b);
    simple_test_assert("Buffer Split Fails",
            buffer_split(&b, ' ' , &ba));

    Buffer *item;

    const char * data[] = { "Hello", "From", "Mars" };

    simple_test_assert("Buffer split malfunction (expected 3 items)",
                       buffer_array_get_buffer_count(&ba) == 3);

    const size_t total = buffer_array_get_buffer_count(&ba);

    fprintf(stderr, "Buffer array counts %zu\n", total);
    fflush(stderr);
    for(size_t i=0; i<total; ++i) {
        Buffer out;
        buffer_init(&out);
        buffer_array_cpy_buffer(&ba, i, &out);
        simple_test_assert("Out buffer should be null terminated",
                buffer_is_null_terminated(&out));

        if(buffer_is_null_terminated(&out)) {

            simple_test_assert("Out buffer is not correct",
                               strcmp((char *) out.data,
                                      data[i]) == 0);
        }

    }

}

void recycler_test(Recycler * recycler) {
    simple_test_assert("Recycler Empty despite recycled memory",
                       recycler->cap > 0);
}

void validate_hashvalue(HashValue *hv) {
    simple_test_assert("Hashvalue Key not nulterminated",
                       buffer_is_null_terminated(&hv->key));

    simple_test_assert("Hashvalue Data not nulterminated",
                       buffer_is_null_terminated(&hv->data));

    simple_test_assert("Hashvalue Key not correct?",
                       strcmp((char *) hv->key.data, "Foo") == 0);

    simple_test_assert("Hashvalue Data not correct?",
                       strcmp((char *) hv->data.data, "Bar") == 0);
}

void hash_value_test(Recycler *recycler) {
    HashValue *hv;
    if (NULL == recycler) {
        hv = malloc(sizeof(HashValue));
    } else {
        hv = recycler_get_exact(recycler, sizeof(HashValue));
    }

    simple_test_assert("Unable to reserve memory for hashvalue",
                       NULL != hv);
    if (NULL == hv) return;

    hashvalue_init(hv);
    hashvalue_assign_recylcer(hv, recycler);

    simple_test_assert("Unable to string copy into hashvalue key",
                       buffer_strcpy(&hv->key, "Foo"));

    simple_test_assert("Unable to string copy into hashvalue value",
                       buffer_strcpy(&hv->data, "Bar"));

    validate_hashvalue(hv);

    HashValue hv2;
    hashvalue_init(&hv2);
    hashvalue_assign_recylcer(&hv2, recycler);
    simple_test_assert("Hashvalue copy fails", hashvalue_cpy(&hv2, hv));

    validate_hashvalue(&hv2);

    BufferArray ba;
    buffer_array_init(&ba);
    buffer_array_assign_recycler(&ba, recycler);

    Buffer bTmp;
    buffer_init(&bTmp);
    buffer_assign_recycler(&bTmp, recycler);
    simple_test_assert("Unable to push hash value into array buffer",
                       buffer_push_bytes(&bTmp, (unsigned char *) hv,
                                         sizeof(HashValue)));

    HashValue *hvTmp = (HashValue *) bTmp.data;

    validate_hashvalue(hvTmp);

    BufferArray bufferArray;
    buffer_array_init(&bufferArray);
    buffer_array_assign_recycler(&bufferArray, recycler);
    simple_test_assert("Unable to push buffer onto bufferarray",
                       buffer_array_push(&bufferArray, &bTmp));


    Buffer *b = buffer_array_get_buffer(&bufferArray, 0);

    simple_test_assert("Unable to retrieve keyvalue value buffer from bufferarray",
                       b != NULL);

    hvTmp = (HashValue *) b->data;

    validate_hashvalue(hvTmp);

    HashTuple ht;

    hashtuple_init(&ht);

    hashtuple_assign_recylcer(&ht, recycler);

    simple_test_assert("failure to add hashvalue to hashtuple",
                       hashtuple_add(&ht, hvTmp));

    simple_test_assert("Incorrect Hash Tuple Count",
                       hashtuple_get_count(&ht) == 1);

    HashValue *hv_b_from_ht = hashtuple_get_hash_value_at_idx(&ht, 0);

    simple_test_assert("Hashtuple returned null pointer when retrieving buffer",
                       hv_b_from_ht != NULL);

    Buffer *nh_b_direct_from_ht_ab = buffer_array_get_buffer(&ht.buffer, 0);

    simple_test_assert("Hashtuple returned null pointer when retrieving"
                       " buffer directly from buffer array within  hashtuple",
                       nh_b_direct_from_ht_ab != NULL);

    simple_test_assert("Internal hashtuple buffer is null",
                       nh_b_direct_from_ht_ab->data != NULL);

    if(NULL == hv_b_from_ht) return;
}

void hash_table_test(Recycler * recycler) {

    HashTable ht;
    hashtable_init(&ht);

    const char* const ary[] = { "foo", "bar", "taco", "beer", 0 }; // all const

    for(size_t i=0; NULL != ary[i]; ++i) {
        Buffer key;
        buffer_init(&key);
        buffer_assign_recycler(&key, recycler);
        simple_test_assert("Failure to strcpy into key buffer",
                buffer_strcpy(&key, ary[i]));

        Buffer value;
        buffer_init(&value);
        buffer_assign_recycler(&value, recycler);

        simple_test_assert("Failure to pushbytes into value buffer",
                           buffer_push_bytes(&value, (unsigned  char *) &i,
                                   sizeof(i)));
        simple_test_assert("Failure to add key/value to hashtable",
                hashtable_add(&ht, &key, &value));

    }

    for(size_t i=0; NULL != ary[i]; ++i) {
        Buffer key;
        buffer_init(&key);
        buffer_assign_recycler(&key, recycler);
        simple_test_assert("Failure to strcpy into key buffer",
                           buffer_strcpy(&key, ary[i]));
        Buffer value;
        buffer_init(&value);
        Buffer * ret = hashtable_get(&ht, &key);
        simple_test_assert("Failure to retrieve key from hashtable",
                           NULL !=ret);
        if(NULL == ret) continue;
        int *j = (int *) ret->data;
        simple_test_assert("Incorrect value retrieved from hashtable",
                           *j !=i);
    }
}

void buffer_cleanse_test(Recycler *recycler) {

    Buffer tmp;
    buffer_init(&tmp);
    buffer_assign_recycler(&tmp, recycler);

    const size_t test_count = 10000;
    const size_t max_char = 127;
    const size_t min_str = 1;
    const size_t max_str = 500;
    time_t t;

    srand((unsigned) time(&t));

    for(size_t i = 0; i< test_count; ++i) {

        buffer_clear(&tmp);

        size_t len = rand() % (max_str - min_str) + min_str;

        if(len < min_str) fprintf(stderr, "%zu is too small!\n", len);
        else if(len > max_str) fprintf(stderr, "%zu is too large!\n", len);

        bool pass = true;

        for(size_t j=0; pass && j<len; ++j) {
            char c = rand() % 127;
            pass = buffer_push_byte(&tmp, c);
        }
        simple_test_assert("Unable to push a byte on to tmp buffer", pass);
        if(!pass) break;

        simple_test_assert("Unable to make string a buffer",
                           buffer_make_string(&tmp));

      //  buffer_cleanse_text(&tmp);
/*
        for(size_t j=0; j<len; ++j) {
            char c = tmp.data[j];

            if(!isalpha(c) || isdigit(c) || c != ' ') pass = false;
        }

        simple_test_assert("Buffer cleans allowed non alpha/num/space data",
                           pass);
  */

    }
}

void all_tests() {
    Recycler recycler;
    recycler_init(&recycler);
    buffer_init_test();
    buffer_reserve_test(NULL);
    buffer_set_test(NULL);
    buffer_transform_test(NULL);
    buffer_array_test(NULL);
    buffer_split_test(NULL);
    hash_table_test(NULL);
    hash_value_test(NULL);
/*
    buffer_cleanse_test(NULL);

    fprintf(stderr, "Begin Tests with Recycler\n");
    buffer_reserve_test(&recycler);
    buffer_set_test(&recycler);
    buffer_transform_test(&recycler);
    buffer_array_test(&recycler);
    recycler_test(&recycler);
    buffer_split_test(&recycler);
    hash_table_test(&recycler);
    hash_value_test(&recycler);
    buffer_cleanse_test(&recycler);
*/

}

int main(int argc, char **argv) {

    log_message("The cake is a lie.");
    simple_test_begin();
    all_tests();
    simple_test_end();
    return simple_test_all_tests_passed();
}
