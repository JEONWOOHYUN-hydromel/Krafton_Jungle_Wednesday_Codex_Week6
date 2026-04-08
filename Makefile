CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -pedantic
TARGET = sql_processor
SRCS = main.c parser.c storage.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

test: $(TARGET)
	./$(TARGET) -f examples/01_insert_woo.sql
	./$(TARGET) -f examples/02_insert_alice.sql
	./$(TARGET) -f examples/03_select_users.sql
	-./$(TARGET) -f examples/04_invalid.sql
	-./$(TARGET) -f examples/05_select_missing.sql
