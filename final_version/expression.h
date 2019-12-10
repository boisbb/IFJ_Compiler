/* expression.h
// název projektu: Implementace překladače imperativního jazyka IFJ19
// tým:            040
// varianta:       II (implementace pomocí tabulky s rozptýlenými položkami)
// autoři:         Boris Burkalo, xburka00, 2BIT
//                 Jiří Herrgott, xherrg00, 2BIT
//                 Jan Klusáček, xklusa14, 2BIT
*/

#include "scanner.h"
#include "strings.h"
#include "error.h"
#include "symtab.h"

#define REALIZE_FUNC 1

typedef struct st_tok_it {
  void *tok_cont;
  Type type;
  int prec_tab_id;
  struct st_tok_it *prev;
  struct st_tok_it *next;
} TermStackIt;

typedef struct st_tok {
  struct st_tok_it *top;
} TermStack;

typedef struct st_id_it {
  void *content;
  Type type;
  struct st_id_it *left;
  struct st_id_it *right;
} IdStackIt;

typedef struct st_id {
  struct st_id_it *top;
} IdStack;

int term_stack_init();

int expression(Token *pre_token, Token *act_tok, hSymtab_it *p_variable, hSymtab *actual_table);

int expression_eval(int fction_switch, int pre_token_switch);

int s_push();

void s_pop();

int ready_to_pop(int fction_switch);

int id_s_push(TermStackIt *term_item);

void id_stack_init();

void id_s_pop();

int check_operators_and_operands_syntax(Type operator, int fction_switch);

int realize_function_call(hSymtab_Func* func_data);
