#ifndef STRINGS_H
#define STRINGS_H

#include <string.h>

// Helping module for dynamic allocation of string types //

typedef struct
{
	char *content;            /// string
	size_t size;                 /// size of strings
	size_t asize;                /// actual size of string
} String;

// Creating an empty dynamic string, the string's size is alligned to 8
int str_init(String *str);

// Set content size, data will be deleted, size is aligned
int str_set_size(String *str, size_t size);

// Adds char to end of string.
int str_pushc(String *str, char new_char);

// Append string
int str_push(String *str, const char *new_str);

// Insert new string to pos
int str_insert(String *str, size_t pos, const char *new_str);

// Delete last char, return that char
char str_pop(String *str);

// Clear content, memory remains allocated
void str_clear(String *str);

// Dealloc memory and set string size to 0
void str_free(String *str);

#endif
