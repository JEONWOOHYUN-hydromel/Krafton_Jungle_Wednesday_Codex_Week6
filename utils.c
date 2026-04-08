#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void safe_copy(char *dest, size_t dest_size, const char *src) {
    if (dest == NULL || dest_size == 0) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    snprintf(dest, dest_size, "%s", src);
}

void trim_whitespace(char *text) {
    char *start;
    char *end;
    size_t length;

    if (text == NULL || text[0] == '\0') {
        return;
    }

    start = text;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    length = strlen(text);
    if (length == 0) {
        return;
    }

    end = text + length - 1;
    while (end >= text && isspace((unsigned char)*end)) {
        *end = '\0';
        if (end == text) {
            break;
        }
        end--;
    }
}

void strip_trailing_semicolon(char *text) {
    size_t length;

    if (text == NULL) {
        return;
    }

    trim_whitespace(text);
    length = strlen(text);
    if (length > 0 && text[length - 1] == ';') {
        text[length - 1] = '\0';
        trim_whitespace(text);
    }
}

int starts_with_ignore_case(const char *text, const char *prefix) {
    size_t index;

    if (text == NULL || prefix == NULL) {
        return 0;
    }

    for (index = 0; prefix[index] != '\0'; index++) {
        if (text[index] == '\0') {
            return 0;
        }
        if (tolower((unsigned char)text[index]) != tolower((unsigned char)prefix[index])) {
            return 0;
        }
    }

    return 1;
}

int equals_ignore_case(const char *left, const char *right) {
    size_t index;

    if (left == NULL || right == NULL) {
        return 0;
    }

    index = 0;
    while (left[index] != '\0' && right[index] != '\0') {
        if (tolower((unsigned char)left[index]) != tolower((unsigned char)right[index])) {
            return 0;
        }
        index++;
    }

    return left[index] == '\0' && right[index] == '\0';
}

int read_text_file(const char *path, char *buffer, size_t buffer_size,
                   char *error_message, size_t error_size) {
    FILE *file;
    size_t bytes_read;
    size_t total_read;

    if (path == NULL || buffer == NULL || buffer_size == 0) {
        snprintf(error_message, error_size, "invalid file read arguments");
        return 0;
    }

    file = fopen(path, "r");
    if (file == NULL) {
        snprintf(error_message, error_size, "failed to open SQL file: %s", path);
        return 0;
    }

    total_read = 0;
    while ((bytes_read = fread(buffer + total_read, 1, buffer_size - total_read - 1, file)) > 0) {
        total_read += bytes_read;
        if (total_read >= buffer_size - 1) {
            fclose(file);
            snprintf(error_message, error_size, "SQL input is too long");
            return 0;
        }
    }

    buffer[total_read] = '\0';
    fclose(file);
    return 1;
}

int join_arguments(int argc, char **argv, int start_index, char *buffer, size_t buffer_size,
                   char *error_message, size_t error_size) {
    int index;
    size_t used;
    int written;

    if (buffer == NULL || buffer_size == 0) {
        snprintf(error_message, error_size, "invalid SQL buffer");
        return 0;
    }

    buffer[0] = '\0';
    used = 0;

    for (index = start_index; index < argc; index++) {
        written = snprintf(buffer + used, buffer_size - used, "%s%s",
                           used == 0 ? "" : " ", argv[index]);
        if (written < 0 || (size_t)written >= buffer_size - used) {
            snprintf(error_message, error_size, "SQL input is too long");
            return 0;
        }
        used += (size_t)written;
    }

    if (buffer[0] == '\0') {
        snprintf(error_message, error_size, "empty SQL input");
        return 0;
    }

    return 1;
}
