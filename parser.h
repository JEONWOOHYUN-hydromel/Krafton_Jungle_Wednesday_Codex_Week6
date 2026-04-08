#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "query.h"

int parse_sql(const char *sql, Query *query, char *error_message, size_t error_size);

#endif
