#ifndef STRINGS_H
#define STRINGS_H

// Helping module for dynamic allocation of string types //

typedef struct
{
	char *content;            /// string
	int size;                 /// size of strings
	int asize;                /// actual size of string
} String;

// Creating an empty dynamic string, the string's size is alligned to 8
int str_init(String *str);

// Adds char to end of string.
int str_add(String *str, char new_char);

//delete last char, return that char
char str_pop(String *str);

// Dealloc memory and set string size to 0
void str_free(String *str);

#endif
