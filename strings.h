#ifndef STRINGS_H
#define STRINGS_H

typedef struct
{
    char *contents;           /// string
    int size;                 /// size of strings
    int asize;                /// actual size of string
} String;

int str_init(String *str);
int str_add(String *str, char new_char);

#endif
