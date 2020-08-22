cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)

add_library(ssc STATIC buffer.h buffer.c recycler.h recycler.c hashtable.h filereader.h hashtable.c filereader.c log.h bufferarray.h bufferarray.c log.c)

