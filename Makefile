CFLAGS=-std=c99 -Wall -Wextra -g
OBJ = scanner.o strings.o
scanner: $(OBJ)
	gcc $(CFLAGS) $(OBJ) -o scanner
scanner.o: scanner.c
	gcc $(CFLAGS) -c scanner.c
strings.o: strings.c strings.h
	gcc $(CFLAGS) -c strings.c
clean:
	rm *.o scanner
