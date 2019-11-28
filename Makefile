CC = gcc
CFLAGS=-std=c99 -Wall -Wextra -g -D DEBUG
OBJ1 = symtab_stack.o strings.o stack.o scanner.o sym_tab.o generator.o test_main.o expression.o
TARGET1 = test_main

$(TARGET1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(TARGET1)

clean:
	rm $(OBJ1) $(TARGET1)
