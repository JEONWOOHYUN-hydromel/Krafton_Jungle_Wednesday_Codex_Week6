#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>

#include "query.h"

int storage_insert(const Query *query, char *error_message, size_t error_size);
int storage_select_all(const Query *query, char *error_message, size_t error_size);

#endif
