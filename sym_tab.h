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
  bool global; // 0 if its not global | 1 if its global
  bool defined;
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
  int paramNum;
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

 //__________________STACK____________
typedef struct fct_def_node {
  char *fct_name;
  int param_num;
  struct fct_def_node *prev;
  struct fct_def_node *next;
} hSym_fct_node;


typedef struct fct_def_stack {
  struct fct_def_node *top;
} hSym_fct_stack;

typedef hSymtab_it *hSymtab[HTAB_PRIME];

int sym_stack_init(hSym_fct_stack* f_stack);

hSym_fct_node *sym_stack_push(hSym_fct_node* f_top, char *name);

int sym_stack_pop_all(hSym_fct_node* f_top, hSymtab* act_table);

typedef hSymtab_it *hSymtab[HTAB_PRIME];

unsigned int symtab_hash_function(const char *str);

void symtab_init(hSymtab *sym_tab);

int symtab_it_get_type(hSymtab_it *symtab_it);

hSymtab_it *symtab_it_position(char *searched_for, hSymtab *sym_tab);

void symtab_add_it(hSymtab *sym_tab, Token *token);

void symtab_add_var_data(hSymtab_it *symtab_it, Type type);

void symtab_add_predef_func(hSymtab *table);

void free_symtab(hSymtab *table, int switch_local);

int is_item_var_defined(char *desired, hSymtab *sym_tab);

int symtab_copy(hSymtab *origin, hSymtab *new);
