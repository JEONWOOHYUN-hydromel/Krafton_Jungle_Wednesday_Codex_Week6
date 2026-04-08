#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stddef.h>

#include "query.h"

int execute_query(const Query *query, char *error_message, size_t error_size);

#endif
