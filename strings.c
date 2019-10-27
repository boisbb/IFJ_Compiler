// Helping module for dynamic allocation of string types //

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "strings.h"

#define STR_SIZE 8


// Creating an empty dynamic string, the string's size is alligned to 8
int str_init(String *str){
  if ((str->contents = malloc(sizeof(char) * STR_SIZE)) == NULL){
    return 0;
  }
  str->size = STR_SIZE;
  str->asize = 0;
  str->contents[str->asize] = '\0';
  return 1;
}

int str_add(String *str, char new_char){

  if (str->size < str->asize) {
    printf("Incorrect values in dyn str\n");
    return 0;
  }

  if (str->size == str->asize) {
    if ((str->contents = realloc(str->contents, str->size + STR_SIZE)) == NULL) {
      printf("Reallocation in str_add FAILED\n");
      return 0;
    }
    str->size += STR_SIZE;
  }
  str->contents[str->asize] = new_char;
  str->asize += 1;
  str->contents[str->asize] = '\0';
  return 1;
}
