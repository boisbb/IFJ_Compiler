#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#include "sym_tab.h"
#include "error.h"
#include "expression.h"
//#include "symtab_stack.h"

extern char current_line[1000];

#define WANTED_TYPE 1
#define UNWANTED_TYPE 0
#define IF_STAT 26
#define WHILE_STAT 27
//#define FCTDEF 28 -> není potřeba, (keyword def)

// FUNCTIONS //
#define LEN 30
#define SUBSTR 31
#define USERDEF 35


int fction_start(Token *token, hSymtab *table);

int fction_params(Token *token, hSymtab_it *symtab_it);

int fction_body(Token *token, hSymtab_it *symtab_it);

int body(Token *token, hSymtab *table);

int fction_call(Token *token, hSymtab *act_table, int in_function);

int statement(Token *token, hSymtab *act_table);

int statement_body(Token *token, hSymtab *act_table);
