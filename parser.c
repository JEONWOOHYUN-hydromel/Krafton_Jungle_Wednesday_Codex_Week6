#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

static void skip_spaces(const char **cursor) {
    while (**cursor != '\0' && isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }
}

static int is_identifier_start(char ch) {
    return isalpha((unsigned char)ch) || ch == '_';
}

static int is_identifier_char(char ch) {
    return isalnum((unsigned char)ch) || ch == '_';
}

static int match_keyword(const char **cursor, const char *keyword) {
    size_t index;
    const char *current;

    current = *cursor;
    for (index = 0; keyword[index] != '\0'; index++) {
        if (current[index] == '\0') {
            return 0;
        }
        if (tolower((unsigned char)current[index]) != tolower((unsigned char)keyword[index])) {
            return 0;
        }
    }

    if (is_identifier_char(current[index])) {
        return 0;
    }

    *cursor = current + index;
    return 1;
}

static int parse_identifier(const char **cursor, char *buffer, size_t buffer_size,
                            char *error_message, size_t error_size) {
    size_t length;
    const char *start;

    if (!is_identifier_start(**cursor)) {
        snprintf(error_message, error_size, "invalid SQL syntax: expected table name");
        return 0;
    }

    start = *cursor;
    length = 0;
    while (is_identifier_char(**cursor)) {
        if (length + 1 >= buffer_size) {
            snprintf(error_message, error_size, "table name is too long");
            return 0;
        }
        buffer[length++] = **cursor;
        (*cursor)++;
    }

    buffer[length] = '\0';
    return start != *cursor;
}

static int append_value_char(char *buffer, size_t buffer_size, size_t *length, char ch,
                             char *error_message, size_t error_size) {
    if (*length + 1 >= buffer_size) {
        snprintf(error_message, error_size, "value is too long");
        return 0;
    }

    buffer[*length] = ch;
    (*length)++;
    buffer[*length] = '\0';
    return 1;
}

static int parse_quoted_value(const char **cursor, char *buffer, size_t buffer_size,
                              char *error_message, size_t error_size) {
    size_t length;

    length = 0;
    (*cursor)++;

    while (**cursor != '\0') {
        if (**cursor == '\'') {
            if ((*cursor)[1] == '\'') {
                if (!append_value_char(buffer, buffer_size, &length, '\'',
                                       error_message, error_size)) {
                    return 0;
                }
                *cursor += 2;
                continue;
            }

            (*cursor)++;
            buffer[length] = '\0';
            return 1;
        }

        if (!append_value_char(buffer, buffer_size, &length, **cursor,
                               error_message, error_size)) {
            return 0;
        }
        (*cursor)++;
    }

    snprintf(error_message, error_size, "invalid SQL syntax: unterminated quoted string");
    return 0;
}

static int parse_unquoted_value(const char **cursor, char *buffer, size_t buffer_size,
                                char *error_message, size_t error_size) {
    size_t length;

    length = 0;
    while (**cursor != '\0' && **cursor != ',' && **cursor != ')') {
        if (!append_value_char(buffer, buffer_size, &length, **cursor,
                               error_message, error_size)) {
            return 0;
        }
        (*cursor)++;
    }

    trim_whitespace(buffer);
    if (buffer[0] == '\0') {
        snprintf(error_message, error_size, "invalid SQL syntax: empty value");
        return 0;
    }

    return 1;
}

static int parse_value_list(const char **cursor, Query *query,
                            char *error_message, size_t error_size) {
    skip_spaces(cursor);
    if (**cursor != '(') {
        snprintf(error_message, error_size, "invalid SQL syntax: expected '(' after VALUES");
        return 0;
    }
    (*cursor)++;

    while (1) {
        skip_spaces(cursor);

        if (**cursor == ')') {
            snprintf(error_message, error_size, "invalid SQL syntax: VALUES list is empty");
            return 0;
        }

        if (query->value_count >= MAX_VALUES) {
            snprintf(error_message, error_size, "too many values in INSERT");
            return 0;
        }

        if (**cursor == '\'') {
            if (!parse_quoted_value(cursor, query->values[query->value_count],
                                    sizeof(query->values[query->value_count]),
                                    error_message, error_size)) {
                return 0;
            }
        } else {
            if (!parse_unquoted_value(cursor, query->values[query->value_count],
                                      sizeof(query->values[query->value_count]),
                                      error_message, error_size)) {
                return 0;
            }
        }

        query->value_count++;
        skip_spaces(cursor);

        if (**cursor == ',') {
            (*cursor)++;
            continue;
        }

        if (**cursor == ')') {
            (*cursor)++;
            break;
        }

        snprintf(error_message, error_size, "invalid SQL syntax: expected ',' or ')'");
        return 0;
    }

    return 1;
}

static int parse_insert_query(const char *sql, Query *query,
                              char *error_message, size_t error_size) {
    const char *cursor;

    cursor = sql;

    if (!match_keyword(&cursor, "INSERT")) {
        snprintf(error_message, error_size, "invalid SQL syntax");
        return 0;
    }

    skip_spaces(&cursor);
    if (!match_keyword(&cursor, "INTO")) {
        snprintf(error_message, error_size, "invalid SQL syntax: expected INTO");
        return 0;
    }

    skip_spaces(&cursor);
    if (!parse_identifier(&cursor, query->table_name, sizeof(query->table_name),
                          error_message, error_size)) {
        return 0;
    }

    skip_spaces(&cursor);
    if (!match_keyword(&cursor, "VALUES")) {
        snprintf(error_message, error_size, "invalid SQL syntax: expected VALUES");
        return 0;
    }

    if (!parse_value_list(&cursor, query, error_message, error_size)) {
        return 0;
    }

    skip_spaces(&cursor);
    if (*cursor != '\0') {
        snprintf(error_message, error_size, "invalid SQL syntax: unexpected trailing text");
        return 0;
    }

    query->type = QUERY_INSERT;
    return 1;
}

static int parse_select_query(const char *sql, Query *query,
                              char *error_message, size_t error_size) {
    const char *cursor;

    cursor = sql;

    if (!match_keyword(&cursor, "SELECT")) {
        snprintf(error_message, error_size, "invalid SQL syntax");
        return 0;
    }

    skip_spaces(&cursor);
    if (*cursor != '*') {
        snprintf(error_message, error_size, "invalid SQL syntax: only SELECT * is supported");
        return 0;
    }
    cursor++;

    skip_spaces(&cursor);
    if (!match_keyword(&cursor, "FROM")) {
        snprintf(error_message, error_size, "invalid SQL syntax: expected FROM");
        return 0;
    }

    skip_spaces(&cursor);
    if (!parse_identifier(&cursor, query->table_name, sizeof(query->table_name),
                          error_message, error_size)) {
        return 0;
    }

    skip_spaces(&cursor);
    if (*cursor != '\0') {
        snprintf(error_message, error_size, "invalid SQL syntax: unexpected trailing text");
        return 0;
    }

    query->type = QUERY_SELECT;
    query->select_all = 1;
    return 1;
}

int parse_sql(const char *sql, Query *query, char *error_message, size_t error_size) {
    char buffer[MAX_SQL_LENGTH];

    if (sql == NULL || query == NULL) {
        snprintf(error_message, error_size, "invalid parser input");
        return 0;
    }

    if (strlen(sql) >= sizeof(buffer)) {
        snprintf(error_message, error_size, "SQL input is too long");
        return 0;
    }

    memset(query, 0, sizeof(*query));
    query->type = QUERY_UNKNOWN;

    safe_copy(buffer, sizeof(buffer), sql);
    trim_whitespace(buffer);
    strip_trailing_semicolon(buffer);

    if (buffer[0] == '\0') {
        snprintf(error_message, error_size, "empty SQL input");
        return 0;
    }

    safe_copy(query->raw_sql, sizeof(query->raw_sql), buffer);

    if (starts_with_ignore_case(buffer, "INSERT")) {
        return parse_insert_query(buffer, query, error_message, error_size);
    }

    if (starts_with_ignore_case(buffer, "SELECT")) {
        return parse_select_query(buffer, query, error_message, error_size);
    }

    snprintf(error_message, error_size, "invalid SQL syntax");
    return 0;
}
