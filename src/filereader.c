//
// Created by Joseph Hurdle on 7/5/20.
//

#include <assert.h>
#include <fcntl.h>
#include "filereader.h"
#include "recycler.h"
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


#define FILE_READER_BUFFER 1024 * 1024
blksize_t BLOCKSIZE;

unsigned long long file_get_blocksize(FileReader *file)
{
    const blksize_t default_buf_size = 1024 * 4; // 4k is a good guess
    struct stat fi;

    if(buffer_is_empty(&file->fileName)) return default_buf_size;
    if(!buffer_make_string(&file->fileName)) return default_buf_size;
    const size_t ret = stat((char *) file->fileName.data, &fi);
    if(0 != ret ) return default_buf_size;
    return fi.st_blksize;
}


void file_reader_init(FileReader * file)
{
    buffer_init(&file->fileName);
    buffer_init(&file->buf);
    file->open = false;
    file->fd = -1;
    file->offset = 0;
    file->eof = false;
    file->recycler = NULL;
}

void file_reader_close(FileReader *file) {

    assert(NULL != file);

    if(!file->open) return;
    buffer_free(&file->buf);
    buffer_free(&file->fileName);
    file->open = false;
    file->fd = -1;
}


bool file_reader_open(FileReader *file, const char *fileName) {

    assert(NULL != file);
    assert(NULL != fileName);

    if(file->open) file_reader_close(file);

    file->fd = open(fileName, O_RDONLY);
    if(file->fd == -1) {
        fprintf(stderr, "Unable to open file [%s] for reading, error [%s]\n",
                fileName, strerror(errno));
        return false;
    }
    buffer_strcpy(&file->fileName, fileName);

    file->open = true;
    return true;
}

const char * file_get_filename(FileReader *file)
{
    if(!buffer_make_string(&file->fileName)) return NULL;
    return (const char *) file->fileName.data;
}

bool file_refill_buffer(FileReader *file) {

    assert(NULL != file);

    if(file->eof) return false;
    if(!file->open) return false;

    Buffer tmp;
    buffer_init(&tmp);
    tmp.recycler = file->recycler;

    const unsigned long long BS = file_get_blocksize(file);

    if(!buffer_reserve(&tmp, BS)) {
        log_message("Unable to reserve a buffer with %zu bytes for file", BS);
        return false;
    }

    // read those bytes
    int ret = read(file->fd, tmp.data, BS);

    // 0 means eof
    if(0 == ret) {
        file->eof = true;
        return false;
    } else if (ret < 0)
    {
        // read error
    }

    // tmp's actual length is equal to count of blocks read
    tmp.len = ret;

    if(buffer_is_empty(&file->buf)) {
        buffer_swap(&file->buf, &tmp);
        file->offset = 0;
        return true;
    }

    // remaining bytes in old buffer
    const long long remainingBytes = (long long) (file->buf.len - file->offset);

    // left shift so only remaining bytes are left in buffer
    if(!buffer_lshift(&file->buf, file->offset)) {
        log_message("Failure to lshift file buffer by %zu before appending %zu bytes",
                    file->offset, BS);
        return false;
    }

    // append the new block of file data to the file's buffer
    if(!buffer_append(&file->buf, &tmp)) {
        log_message("Failure to append temporary read buffer to file buffer");
        return false;
    }

    // return tmp now that we are done
    buffer_free(&tmp);

    return true;
}

bool file_reader_read_byte(FileReader *file, unsigned char *byte) {
    assert(NULL != file);
    assert(NULL != byte);

    if(file->offset >= file->buf.len) {
        if(!file_refill_buffer(file)) {
            if(file_reader_eof(file)) return false;  // handle eof silently
            log_message("failure to refill file buffer, unable to read more");
            return false;
        }
        return file_reader_read_byte(file, byte);
    }

    *byte = file->buf.data[file->offset++];
    return true;
}

bool file_reader_read_line(FileReader *file, Buffer *buf, unsigned char delim) {

    assert(NULL != file);
    assert(NULL != buf);

    bool delimFound = false;
    bool error = false;

    unsigned char c;

    buffer_clear(buf);

    while(!delimFound) {

        if(!file_reader_read_byte(file, &c)) {
            if(file->eof) {
                if(buffer_is_empty(buf)) return false;
                return true;
            }
        }

        if(delim == c) delimFound = true;
        if(!buffer_push_byte(buf, c)) {
            log_message("unable to push a byte on output line buffer");
            return false;
        }
    }

    return true;
}

bool file_reader_eof(FileReader *file) {
    assert(NULL != file);
    return file->eof;
}

void file_reader_assign_recycler(FileReader *file, Recycler *rc) {
    assert(NULL != file);
    assert(NULL != rc);
    file->recycler = rc;
    file->fileName.recycler = rc;
    file->buf.recycler = rc;
}