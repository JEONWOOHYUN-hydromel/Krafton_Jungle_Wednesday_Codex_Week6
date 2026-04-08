#include <stdio.h>
#include <string.h>

#include "executor.h"
#include "parser.h"
#include "query.h"
#include "utils.h"

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
