#ifndef TB_FILE_H
#define TB_FILE_H

#include <stdio.h>

#define TB_FILE_COPY_BUFFER_SIZE    4096

char* tb_file_read(const char* path, const char* mode);
char* tb_file_read_alloc(const char* path, const char* mode, void* (*mallocf)(size_t), void (*freef)(void*));

char* tb_file_readf(FILE* const stream);
char* tb_file_readf_alloc(FILE* const stream, void* (*mallocf)(size_t), void (*freef)(void*));

size_t tb_file_read_buf(const char* path, const char* mode, char* buf, size_t max_len);

/* Copies a file from src_path to dst_path without dynamic memory allocations. */
size_t tb_file_copy(const char* src_path, const char* dst_path);

#endif /* !TB_FILE_H */