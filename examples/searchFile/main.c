#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include "../../src/hashtable.h"
#include "../../src/filereader.h"
#include "../../src/recycler.h"


// find an argment named [arg_name] in [argv] using [argc] as length of argv.
// [argc] - number of arguments
// [argv] - array of argument pointers
// [arg_name] - name of argument searched for
// returns pointer to array of string containing argument value of arg_name
const char *get_arg(int argc, const char **argv, const char *arg_name) {
    assert(NULL != argv);
    assert(NULL != arg_name);

    for (int i = 0; i < argc; ++i) {
        if (strcmp(arg_name, argv[i]) == 0) {
            if ((i + 1) < argc) return argv[i + 1];
        }
    }
    return NULL;
}

// dump usage screen given name of command executed [cmd]
// [cmd] - name of command executed
void usage(const char *cmd) {
    fprintf(stderr, "%s -dict [dictionary of words]"\
        " -doc [document file to search]\n", cmd);
}

// parse arguments in [argv] using [argc] count of arguments and set them in [dictFile and [docFile]
// [argc] - number of arguments
// [argv] - array of argument pointers
// [dictFile] - buffer containing name of dictFile (null terminated)
// [docFile] - buffer containing name of dictFile (null terminated)
bool parse_args(int argc, const char **argv, Buffer *dictFile, Buffer *docFile) {

    const char *c = get_arg(argc, argv, "-dict");
    if (NULL != c) buffer_strcpy(dictFile, c);

    const char *d = get_arg(argc, argv, "-doc");
    if (NULL != d) buffer_strcpy(docFile, d);

    bool success = true;

    if (buffer_is_empty(dictFile)) {
        fprintf(stderr, "Missing Required Flag -dict\n");
        success = false;
    }
    if (buffer_is_empty(docFile)) {
        fprintf(stderr, "Missing Required Flag -doc\n");
        success = false;
    }
    if (false == success) {
        usage(argv[0]);
        return false;
    }
    return true;
}

bool count_words(HashTable *dict, Buffer *docFile) {
    FileReader doc;
    file_reader_init(&doc);

    if (!buffer_make_string(docFile)) {
        log_message("cannot convert filename to a string");
        return false;
    }

    file_reader_open(&doc, (char *) docFile->data);

    const size_t wordCount = hashtable_get_entry_count(dict);
    if (0 == wordCount) {
        log_message("empty dictionary");
        return false;
    }

    Buffer line;
    BufferArray tokens;
    buffer_init(&line);
    buffer_array_init(&tokens);

    Buffer token;
    buffer_init(&token);

    size_t line_count = 0;

    bool error = false;
    bool eof = false;

    while (!error && !eof) {

        ++line_count;
        buffer_array_free(&tokens);
        buffer_array_init(&tokens);

        if (file_reader_read_line(&doc, &line, '\n')) {
            buffer_cleanse_text(&line);
            if (buffer_split(&line, ' ', &tokens)) {
                const size_t count = buffer_array_get_buffer_count(&tokens);
                for (size_t i = 0; i < count; ++i) {

                    //   if(NULL != hashtable_get(dict, &token)) we->count++;
                }
            } else {
                log_message("tokenization failed on line %d", line_count);
                eof = true;
            }
        } else {
            if (file_reader_eof(&doc)) eof = true;
            else {
                log_message("error reading from file [%s] line [%d]", docFile->data,
                            line_count);
                error = true;
            }
        }
    }

    file_reader_close(&doc);

    buffer_array_free(&tokens);
    buffer_array_init(&tokens);
    buffer_free(&token);
    return error;
}

bool load_hashtable(HashTable *ht, Buffer *dictFile) {

    hashtable_init(ht);

    FileReader reader;
    file_reader_init(&reader);
    if(!file_reader_open(&reader, buffer_get_string(dictFile)))
    {
        log_message("Unable to open dictionary file");
        return false;
    }

    Buffer line;
    buffer_init(&line);
    line.recycler = ht->recycler;

    if(!file_reader_read_line(&reader, &line, '\n')) {
        log_message("Unable to read first line from file");
        file_reader_close(&reader);
        return false;
    }

    const size_t default_count = 0;
    Buffer value;
    buffer_init(&value);
    if(!buffer_push_bytes(&value, (unsigned char *) &default_count,
                          sizeof(size_t))) {
        log_message("Failure to push default count onto hashvalue");
        file_reader_close(&reader);
        return false;
    }


    while(!buffer_is_empty(&line)) {
        buffer_cleanse_text(&line);
        if(!hashtable_add(ht, &line, &value)) {
            buffer_free(&value);
            buffer_free(&line);
            log_message("Failure to push default count onto hashvalue");
            file_reader_close(&reader);
            return false;
        }

        if(!file_reader_read_line(&reader, &line, '\n')) {
            break;
        }

    }


    return true;
}


int main(int argc, const char **argv) {

    Buffer dictFile, docFile;
    buffer_init(&dictFile);
    buffer_init(&docFile);

    Recycler recycler;
    buffer_assign_recycler(&dictFile, &recycler);
    buffer_assign_recycler(&docFile, &recycler);

    if (!parse_args(argc, argv, &dictFile, &docFile)) return 5;

    HashTable dict;
    hashtable_init(&dict);
    hashtable_assign_recycler(&dict, &recycler);

    if (!load_hashtable(&dict, &dictFile)) return 5;

    if (!count_words(&dict, &docFile)) return 5;

    return 0;
}
