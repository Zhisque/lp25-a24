#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stddef.h>

int is_directory(const char *path);
void list_files(const char *path);
char *read_file(const char *filepath);
void write_file(const char *filepath, const void *data, size_t size);

#endif // FILE_HANDLER_H

