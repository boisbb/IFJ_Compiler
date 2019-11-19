#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "sym_tab.h"
#include "error.h"

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

#define GET_TOKEN_CHECK_EOF(token) \
            (get_next_token(token) == EOF) ? 1 : 0 \

#define TOKEN_TYPE_NEEDED_CHECK(reached_type, needed_type) \
            (needed_type == reached_type) ? 1 : 0 \


#define TOKEN_CHECK_STAT(token) ({ \
            needed_type = UNWANTED_TYPE; \
            if (token->type == TypeKeyword && strcmp(token->data, "if")) { \
              needed_type = IF_STAT; \
            } \
            else if (token->type == TypeKeyword && strcmp(token->data, "while")){ \
              needed_type = WHILE_STAT; \
            } \
            else if (token->type == TypeKeyword && strcmp(token->data, "def")) { \
              needed_type = FCTDEF; \
            } \
            else { \
              needed_type = UNWANTED_TYPE; \
            } \
          }) \

#define WHICH_PREDEF_FCTION(token) \
            (!(strcmp((char*)token->data, "len"))) ? LEN : ((!(strcmp((char*)token->data, "substr"))) ? SUBSTR : USERDEF)  \



#define TOKEN_CHECK_DATA_TYPE(token)({ \
          needed_type = UNWANTED_TYPE; \
          if (token->type == TypeString || token->type == TypeInt || token->type == TypeFloat) { \
            needed_type = (token->type == TypeString) ? TypeString : ((token->type == TypeInt) ? TypeInt : TypeFloat); \
          } \
          else { \
            needed_type = UNWANTED_TYPE; \
          } \
        }) \



int err = 0;
extern hSymtab *table = NULL;

int hovno(){}

void assignment(Token *var, Token *value){
  if (GET_TOKEN_CHECK_EOF(value) || TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  // TODO if the variable has already been added to symtab

  hSymtab_it *tmp_it;
  switch(value->type){
    // Add variable as Integer
    case TypeInt:
      symtab_add_it(table, var);
      symtab_add_var_data((*table)[symtab_hash_function((char*)var->data)], TypeInt);
      break;
    // Add variable as String
    case TypeString:
      symtab_add_it(table, var);
      symtab_add_var_data((*table)[symtab_hash_function((char*)var->data)], TypeString);
      break;
    // Add variable as Float
    case TypeFloat:
      symtab_add_it(table, var);
      symtab_add_var_data((*table)[symtab_hash_function((char*)var->data)], TypeFloat);
      break;
    // Add variable as same type as Variable
    case TypeVariable:
      if (!(tmp_it = symtab_it_position((char*)value->data, table))) {
        DEBUG_PRINT("SYNTAX ERROR: The variable %s is non-existent.\n", (char*)value->data);
        exit(1);
      }
      symtab_add_it(table, var);
      symtab_add_var_data((*table)[symtab_hash_function((char*)var->data)], ((hSymtab_Var*)(tmp_it->data))->type);
      break;

    default:
      break;

  }
}


bool command(Token *token){
  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {

    Token *token_n = malloc(sizeof(Token));
    if (GET_TOKEN_CHECK_EOF(token_n)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}

    if (TOKEN_TYPE_NEEDED_CHECK(token_n->type, TypeAssignment)) {

      free(token_n->data);
      assignment(token, token_n);


    }
  }
  return false;
}

void body(Token *token){
  command(token);
  if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return;}
  while (1) {
      if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {

        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return;}
        command(token);
      }
      else{
        DEBUG_PRINT("Found EOF, parsing terminated.\n");
        return;
      }

  }
}

int fction_params(Token *token){
  //need new token, now token is left bracket
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
  int check_comma = 0; //params should not end with comma
  while(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)){
    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
        printf("var\n");
        check_comma = 0;
      //zapsat parametry do tabulky bez typu
      /*
      hSymtab_Func_Param *func_param;
      *func_param = (hSymtab_Func_Param) malloc(sizeof(fct_param));
      */
    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeComma)){
      printf("comma\n");
      check_comma = 1;
    }
    else{
      return 1;
    }
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
  }
  if (check_comma == 1){
    printf("chyba\n");
    return 1;
  }
  else{
    return 0;
  }
}

int fction_start(Token *token){
  //return 1 = error
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  //beginning of function
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
    Token *fction_name = token;

    //check if next token is left bracket and not EOF
    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeLeftBracket)){
      //check if function with same name already exists
      if(symtab_it_position((char*)token->data, table) == NULL){
        //printf("dobrý\n");
        fction_name->type = TypeFunc;
        symtab_add_it(table, fction_name); //add name of fction to table
        if(fction_params(token) != 0){ //pořešit návratové hodnoty
          return 1;
        }
        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
        if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)){
          printf("spravně\n");
        }
        //fction_body();

      }
      else{
        return 1; //function already exists
      }
    }
  }
}

// FOR NOW EQUAL TO <prog>
// <prog> -> <body>
// <prog> -> <fction_start> INDENT <body> DEDENT <body>
int prog() {
  Token *token = malloc(sizeof(Token));
  scanner_init();


  if(GET_TOKEN_CHECK_EOF(token)){
    DEBUG_PRINT("Test file is empty.\n");
    return 1;
  }

  table = malloc(sizeof(hSymtab));
  symtab_init(table);
  symtab_add_predef_func(table);




  if (strcmp((char*)token->data, "def") == 0) {
    fction_start(token); //kontrolovat, co to vrátilo, kvůli returnům
    return 0;
  }
  else {
    body(token);
  }

  return 0;
}
