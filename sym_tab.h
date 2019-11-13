#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "strings.h"
#include "scanner.h"

#define HTAB_PRIME 24593

// Symtable item data
typedef struct symtab_it_data {
  int global; // 0 if its not global | 1 if its global
    Type type;
  int value_int;
  char *value_str;
  float value_float;
  bool value_bool;
} hSymtab_data;

// Symtable item
typedef struct symtab_it {
  char *hKey;            // based on key the item is placed in htab
  hSymtab_data *data;
  struct symtab_it *next;
} hSymtab_it;

typedef hSymtab_it *hSymtab[HTAB_PRIME];

unsigned int symtab_hash_function(const char *str);

void symtab_init(hSymtab *sym_tab);

hSymtab_it *symtab_it_position(char *searched_for, hSymtab *sym_tab);

void symtab_add_it(hSymtab *sym_tab, Token *token);

void symtab_add_value(hSymtab *sym_tab, Token *token_var,Token *token_val);
