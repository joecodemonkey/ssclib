# ssclib

A simple c library that attempts to make development in c safer and simpler

# **WARNING: The state of this library is pre-alpha.  It’s not ready for use.**

# Features

This library includes several useful datatypes that wrap a lot of the repetative
work that is required to do anything in c.  My goal isn't to be flashy, it's
to provide a safe library.

Every datatype has an _init method which should be used to prepare it for use.

Every datatype has a _free method which should be used to free its internally
held data.

When I previously wrote a lot of C++, I used a recycler to great effect, 
speeding up memory allocation and deallocation by preventing calls to free.
I’ve revisited this concept and it is available for every datatype in this
library.  Without template programming, it isn’t as efficient.  Also, it’s
pretty half baked. It may end up being removed if I profiling demonstrates that
it doesn’t help.


## Recycler

Rather than rewriting malloc, Recycler provides a shim which may be used to
manage memory.

``` c
    Recycler recycler;
    recycler_init(&recycler);
    MemoryChunk chunk;
    // recycler will either return a buffer from its internal store
    // or, it will malloc the memory on your behalf
    if(!recycler_get(&recycler, &chunk, 1024)) {
        log_message("Error allocating buffer");
        return;
    }

    // gauranteed to be >= 1024 bytes
    chunk.cap;
    // pointer to data
    chunk.p;

    // will free memory or, add it to free list inside of recycler   
    recycler_return(&recycler, cunk.cap, chunk.p);

```

## Buffer

A managed array of bytes which provides various bits of useful functionality.

``` c
    Buffer b;

    // always call init on data structures in this library to prevent undefined 
    // behavior
    buffer_init(&b);

    // optoinally assign a recycler (from above)
    // If you don't do it, free and malloc will be called on your behalf
    buffer_assign_recycler(&b, &recycler);

    // reserve 50 bytes within the buffer
    buffer_reserve(&b, 50)

    const char * s = "THE CAKE IS A LIE.";

    // Push the text in s onto buffer b.  Note that if you don't preallcoate
    // memory to the buffer, the buffer will expand itself on your behalf

    if(!buffer_push_bytes(&b, s, strlen(s) )) {
        log_message("Unable to push %zu bytes onto buffer", strlen(s));
        return;
    }
    
    buffer_downcase(&b);

    printf("%s\n", buffer_get_string(&b)); // the cake is a lie

    // always call the _free method when you are done with a data structure 
    // from this library.  the buffer will use the recycler to return the
    // memory if you assigned one or, will use free if you didn't.
    buffer_free(&b);

```

## BufferArray

An array of buffers.

``` c

    BufferArray ba;

    // always call init on data structures in this library to prevent undefined 
    // behavior    
    buffer_array_init(&ba);

    // assign a recycler as created earlier in the recycler example
    buffer_array_assign_recycler(&ba, &recycler);

    for(size_t i=0; i<10; ++i) {
        Buffer b;
        buffer_init(&b);
        buffer_assign_recycler(&b, &recycler);
        
    }
    
    // always call the _free method when you are done with a data structure 
    // from this library.  the buffer will use the recycler to return the
    // memory if you assigned one or, will use free if you didn't.

```

## FileRead

A reader which uses an internal buffer to speed up reads.

``` c

    FileReader reader;
    file_reader_init(&reader);
    file_reader_assign_recycler(&reader, &recycler);

    if(!file_reader_open(&reader, "cake.txt"))
    {
        log_message("Unable to open cake.txt file");
        return;
    }

    Buffer line;
    buffer_init(&line);

    // read a line using a delimter
    if(!file_reader_read_line(&reader, &line, '\n')) {
        log_message("Unable to read first line from file");
        file_reader_close(&reader);
        return;
    }

    // do something with line
    printf("%s\n", buffer_get_string(&buffer));
    
    // close will close the file and free internal memory
    file_reader_close(&reader);
    buffer_free(&bufer);
```


## HashTable
A very simplistic hashtable.

``` c
    HashTable ht;
    hashtable_init(&ht);

    Buffer key;
    buffer_init(&key);
    buffer_strcpy(&key, "cake");

    Buffer value;
    buffer_init(&value);
    buffer_strcpy(&value, "is a lie");
    
    hashtable_add(&ht, &key, &value);
    
    Buffer * ret = hashtable_get(&ht, &key);
    
    if(NULL == ret) {
        // key not found
    }    
    
    printf("%s", buffer_make_string(ret)); // is a lie

```


## Log
A super simple logger which writes to stderr.

``` c
    log_message("The Cake is a lie"); // ssclib/test/test.c:405 [The cake is a lie.]

```


