CC = gcc
CFLAGS=-std=c99 -Wall -Wextra -g
OBJ1 = test_main.o scanner.o strings.o stack.o sym_tab.o
TARGET1 = test_main

$(TARGET1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(TARGET1)

clean:
	rm $(OBJ1) $(TARGET1)
