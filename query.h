#ifndef QUERY_H
#define QUERY_H

#define MAX_SQL_LENGTH 4096
#define MAX_TABLE_NAME_LENGTH 64
#define MAX_VALUES 32
#define MAX_VALUE_LENGTH 256
#define MAX_PATH_LENGTH 256
#define DATA_DIRECTORY "data"

typedef enum {
    QUERY_INSERT = 0,
    QUERY_SELECT,
    QUERY_UNKNOWN
} QueryType;

typedef struct {
    QueryType type;
    char table_name[MAX_TABLE_NAME_LENGTH];
    char values[MAX_VALUES][MAX_VALUE_LENGTH];
    int value_count;
} Query;

#endif
