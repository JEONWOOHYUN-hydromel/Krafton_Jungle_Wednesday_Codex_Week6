#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "query.h"
#include "storage.h"

static int read_text_file(const char *path, char *buffer, size_t buffer_size,
                          char *error_message, size_t error_size) {
    FILE *file;
    size_t total_read;
    size_t bytes_read;

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

static int join_arguments(int argc, char **argv, int start_index, char *buffer, size_t buffer_size,
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

static int execute_query(const Query *query, char *error_message, size_t error_size) {
    if (query == NULL) {
        snprintf(error_message, error_size, "invalid query");
        return 0;
    }

    if (query->type == QUERY_INSERT) {
        if (!storage_insert(query, error_message, error_size)) {
            return 0;
        }
        printf("[OK] inserted into %s\n", query->table_name);
        return 1;
    }

    if (query->type == QUERY_SELECT) {
        return storage_select_all(query, error_message, error_size);
    }

    snprintf(error_message, error_size, "unsupported query type");
    return 0;
}

static void print_usage(const char *program_name) {
    printf("Usage:\n");
    printf("  %s \"INSERT INTO users VALUES (1, 'woo');\"\n", program_name);
    printf("  %s -f examples/03_select_users.sql\n", program_name);
}

int main(int argc, char **argv) {
    char sql[MAX_SQL_LENGTH];
    char error_message[256];
    Query query;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-f") == 0) {
        if (argc < 3) {
            fprintf(stderr, "[ERROR] missing SQL file path\n");
            print_usage(argv[0]);
            return 1;
        }

        if (!read_text_file(argv[2], sql, sizeof(sql), error_message, sizeof(error_message))) {
            fprintf(stderr, "[ERROR] %s\n", error_message);
            return 1;
        }
    } else {
        if (!join_arguments(argc, argv, 1, sql, sizeof(sql), error_message, sizeof(error_message))) {
            fprintf(stderr, "[ERROR] %s\n", error_message);
            return 1;
        }
    }

    if (!parse_sql(sql, &query, error_message, sizeof(error_message))) {
        fprintf(stderr, "[ERROR] %s\n", error_message);
        return 1;
    }

    if (!execute_query(&query, error_message, sizeof(error_message))) {
        fprintf(stderr, "[ERROR] %s\n", error_message);
        return 1;
    }

    return 0;
}
