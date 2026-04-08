#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void safe_copy(char *dest, size_t dest_size, const char *src);
void trim_whitespace(char *text);
void strip_trailing_semicolon(char *text);
int starts_with_ignore_case(const char *text, const char *prefix);
int equals_ignore_case(const char *left, const char *right);
int read_text_file(const char *path, char *buffer, size_t buffer_size,
                   char *error_message, size_t error_size);
int join_arguments(int argc, char **argv, int start_index, char *buffer, size_t buffer_size,
                   char *error_message, size_t error_size);

#endif
