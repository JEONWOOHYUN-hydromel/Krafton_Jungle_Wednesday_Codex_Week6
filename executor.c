#include "executor.h"

#include <stdio.h>

#include "storage.h"

int execute_query(const Query *query, char *error_message, size_t error_size) {
    if (query == NULL) {
        snprintf(error_message, error_size, "invalid query");
        return 0;
    }

    switch (query->type) {
        case QUERY_INSERT:
            if (!storage_insert(query, error_message, error_size)) {
                return 0;
            }
            printf("[OK] inserted into %s\n", query->table_name);
            return 1;

        case QUERY_SELECT:
            return storage_select_all(query, error_message, error_size);

        case QUERY_UNKNOWN:
        default:
            snprintf(error_message, error_size, "unsupported query type");
            return 0;
    }
}
