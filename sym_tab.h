#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "strings.h"
#include "scanner.h"
#include "error.h"

#define HTAB_PRIME 24593

// Symtable item data
typedef struct symIt_Var {
  int global; // 0 if its not global | 1 if its global
  Type type;

} hSymtab_Var;

typedef struct fct_param {
  Type param_type;
  char *paramName;
  struct fct_param *next;
} hSymtab_Func_Param;

typedef struct symIt_Func {
  bool defined;
  hSymtab_Func_Param *params;
  Type return_type;
} hSymtab_Func;

typedef enum {
  IT_VAR,
  IT_FUNC
} IT_Type;

// Symtable item
typedef struct symtab_it {
  char *hKey;            // based on key the item is placed in htab
  IT_Type item_type;
  void *data;
  struct symtab_it *next;
} hSymtab_it;

typedef hSymtab_it *hSymtab[HTAB_PRIME];

unsigned int symtab_hash_function(const char *str);

void symtab_init(hSymtab *sym_tab);

int symtab_it_get_type(hSymtab_it *symtab_it);

hSymtab_it *symtab_it_position(char *searched_for, hSymtab *sym_tab);

void symtab_add_it(hSymtab *sym_tab, Token *token);

void symtab_add_var_data(hSymtab_it *symtab_it, Type type);

void symtab_add_predef_func(hSymtab *table);
