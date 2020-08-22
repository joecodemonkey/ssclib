//
// Created by Joseph Hurdle on 7/5/20.
//

#ifndef SEARCHFILEC_FILEREADER_H
#define SEARCHFILEC_FILEREADER_H

#include <stdbool.h>
#include "buffer.h"
#include "recycler.h"

typedef struct stFileReader {
    Buffer fileName;
    bool open;
    int fd;
    Buffer buf;
    size_t offset;
    bool eof;
    Recycler *recycler;

} FileReader;


void file_reader_init(FileReader * file);
void file_reader_close(FileReader *file);
bool file_reader_open(FileReader *file, const char *fileName);
bool file_reader_read_byte(FileReader *file, unsigned char *byte);
bool file_reader_read_line(FileReader *file, Buffer *buf, unsigned char delim);
bool file_reader_eof(FileReader *file);
void file_reader_assign_recycler(FileReader *file, Recycler *rc);


#endif //SEARCHFILEC_FILEREADER_H
