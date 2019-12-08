#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#include "sym_tab.h"
#include "error.h"
#include "expression.h"
//#include "symtab_stack.h"

#define WANTED_TYPE 1
#define UNWANTED_TYPE 0
#define IF_STAT 26
#define WHILE_STAT 27
//#define FCTDEF 28 -> není potřeba, (keyword def)

// FUNCTIONS //
#define LEN 30
#define SUBSTR 31
#define USERDEF 35


// ______Print Stack_______ //
typedef struct pr_Stack {
  Token *term;
  int top;
  unsigned size;
} Print_Stack;

int print_fct_call(Token *token, hSymtab *act_table, int in_function);
//int print_stack_push(Token *token);

//int print_stack_pop_all();
//_______________________________

int fction_start(Token *token, hSymtab *table);

int fction_params(Token *token, hSymtab_it *symtab_it);

int fction_body(Token *token, hSymtab_it *symtab_it);

int body(Token *token, hSymtab *table);

int fction_call(Token *token, hSymtab *act_table, int in_function);

int statement(Token *token, hSymtab *act_table, int in_function, char* fction_name, size_t pos);

int statement_body(Token *token, hSymtab *act_table, int in_function, char* fction_name, size_t pos);

int command(Token *token, hSymtab *act_table, int in_function, char* fction_name, int statement_switch, size_t pos);

int prog();

int assignment(Token *var, Token *value, hSymtab *act_table, int in_function, size_t pos);
