CC = gcc
CFLAGS=-std=c99 -Wall -Wextra -g -D DEBUG
OBJ1 = parser.o strings.o stack.o scanner.o symtab.o generator.o expression.o main.o
TARGET1 = main

$(TARGET1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(TARGET1)

clean:
	rm $(OBJ1) $(TARGET1)
