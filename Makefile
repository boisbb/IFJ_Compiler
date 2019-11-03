CC = gcc
CFLAGS=-std=c99 -Wall -Wextra -g
OBJ = scanner.o strings.o stack.o
TARGET = scanner

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)
clean:
	rm $(OBJ) $(TARGET)
