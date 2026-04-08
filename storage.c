#include "storage.h"

#include <stdio.h>
#include <string.h>

static void build_table_path(const char *table_name, char *path, size_t path_size) {
    snprintf(path, path_size, "%s/%s.csv", DATA_DIRECTORY, table_name);
}

static int write_csv_field(FILE *file, const char *value) {
    size_t index;
    int needs_quotes;

    needs_quotes = 0;
    for (index = 0; value[index] != '\0'; index++) {
        if (value[index] == ',' || value[index] == '"' ||
            value[index] == '\n' || value[index] == '\r') {
            needs_quotes = 1;
            break;
        }
    }

    if (!needs_quotes) {
        return fprintf(file, "%s", value) >= 0;
    }

    if (fputc('"', file) == EOF) {
        return 0;
    }

    for (index = 0; value[index] != '\0'; index++) {
        if (value[index] == '"') {
            if (fputc('"', file) == EOF) {
                return 0;
            }
        }
        if (fputc(value[index], file) == EOF) {
            return 0;
        }
    }

    return fputc('"', file) != EOF;
}

static int parse_csv_line(const char *line, char fields[][MAX_VALUE_LENGTH], int max_fields,
                          char *error_message, size_t error_size) {
    int field_count;
    size_t field_length;
    int in_quotes;
    const char *cursor;

    field_count = 0;
    field_length = 0;
    in_quotes = 0;
    cursor = line;

    if (*cursor == '\0' || *cursor == '\n' || *cursor == '\r') {
        return 0;
    }

    if (field_count >= max_fields) {
        snprintf(error_message, error_size, "too many columns in CSV row");
        return -1;
    }
    fields[field_count][0] = '\0';

    while (*cursor != '\0') {
        char current = *cursor;

        if (in_quotes) {
            if (current == '"') {
                if (cursor[1] == '"') {
                    if (field_length + 1 >= MAX_VALUE_LENGTH) {
                        snprintf(error_message, error_size, "CSV field is too long");
                        return -1;
                    }
                    fields[field_count][field_length++] = '"';
                    fields[field_count][field_length] = '\0';
                    cursor += 2;
                    continue;
                }
                in_quotes = 0;
                cursor++;
                continue;
            }

            if (field_length + 1 >= MAX_VALUE_LENGTH) {
                snprintf(error_message, error_size, "CSV field is too long");
                return -1;
            }
            fields[field_count][field_length++] = current;
            fields[field_count][field_length] = '\0';
            cursor++;
            continue;
        }

        if (current == '"') {
            in_quotes = 1;
            cursor++;
            continue;
        }

        if (current == ',') {
            field_count++;
            if (field_count >= max_fields) {
                snprintf(error_message, error_size, "too many columns in CSV row");
                return -1;
            }
            field_length = 0;
            fields[field_count][0] = '\0';
            cursor++;
            continue;
        }

        if (current == '\n' || current == '\r') {
            break;
        }

        if (field_length + 1 >= MAX_VALUE_LENGTH) {
            snprintf(error_message, error_size, "CSV field is too long");
            return -1;
        }
        fields[field_count][field_length++] = current;
        fields[field_count][field_length] = '\0';
        cursor++;
    }

    if (in_quotes) {
        snprintf(error_message, error_size, "malformed CSV row");
        return -1;
    }

    return field_count + 1;
}

int storage_insert(const Query *query, char *error_message, size_t error_size) {
    char path[MAX_PATH_LENGTH];
    FILE *file;
    int index;

    build_table_path(query->table_name, path, sizeof(path));

    file = fopen(path, "r");
    if (file == NULL) {
        snprintf(error_message, error_size, "table file not found: %s", path);
        return 0;
    }
    fclose(file);

    file = fopen(path, "a");
    if (file == NULL) {
        snprintf(error_message, error_size, "failed to open table file for append: %s", path);
        return 0;
    }

    for (index = 0; index < query->value_count; index++) {
        if (!write_csv_field(file, query->values[index])) {
            fclose(file);
            snprintf(error_message, error_size, "failed to write CSV data");
            return 0;
        }
        if (index + 1 < query->value_count && fputc(',', file) == EOF) {
            fclose(file);
            snprintf(error_message, error_size, "failed to write CSV separator");
            return 0;
        }
    }

    if (fputc('\n', file) == EOF) {
        fclose(file);
        snprintf(error_message, error_size, "failed to write CSV newline");
        return 0;
    }

    fclose(file);
    return 1;
}

int storage_select_all(const Query *query, char *error_message, size_t error_size) {
    char path[MAX_PATH_LENGTH];
    char line[MAX_SQL_LENGTH];
    char fields[MAX_VALUES][MAX_VALUE_LENGTH];
    FILE *file;

    build_table_path(query->table_name, path, sizeof(path));

    file = fopen(path, "r");
    if (file == NULL) {
        snprintf(error_message, error_size, "table file not found: %s", path);
        return 0;
    }

    printf("[RESULT] %s\n", query->table_name);

    while (fgets(line, sizeof(line), file) != NULL) {
        int field_count;
        int index;

        field_count = parse_csv_line(line, fields, MAX_VALUES, error_message, error_size);
        if (field_count < 0) {
            fclose(file);
            return 0;
        }
        if (field_count == 0) {
            continue;
        }

        for (index = 0; index < field_count; index++) {
            printf("%s", fields[index]);
            if (index + 1 < field_count) {
                printf(",");
            }
        }
        printf("\n");
    }

    fclose(file);
    return 1;
}
