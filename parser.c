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
#define FCTDEF 28

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



#define TOKEN_CHECK_DATA_TYPE(token)({ \
          needed_type = UNWANTED_TYPE; \
          if (token->type == TypeString || token->type == TypeInt || token->type == TypeFloat) { \
            needed_type = (token->type == TypeString) ? TypeString : ((token->type == TypeInt) ? TypeInt : TypeFloat); \
          } \
          else { \
            needed_type = UNWANTED_TYPE; \
          } \
        }) \


#define SYMTAB_ADD_VAR(sym_tab, token) \
          if (token->type == TypeVariable && token->data != NULL) { \
            symtab_add_it(sym_tab, token); \
          } \


#define SYMTAB_ADD_VALUE(sym_tab, token, token_n) \
          if ((*sym_tab)[symtab_hash_function((char*)token->data)] != NULL && token != NULL && token_n != NULL) { \
            symtab_add_value(sym_tab, token, token_n); \
          } \


int err = 0;
extern hSymtab *table = NULL;

void value(Token *token){
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  if (token->type == TypeString) {
  }

  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt)
      || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeString)
      || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeFloat)
      || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {
        return;
  }

}

int fction(Token *token){
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  //tady bude muset být cyklus, parametrů může být více
  if(token->type == TypeVariable){
    printf("promenna\n");
  }
  //toto bude konec cyklu
  if(token->type == TypeRightBracket){
    printf("konec\n");
  }

  //pak bude asi funkce fction_body
}


//nemůže být bool kvůli mallocu
int command(Token *token){
  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {

    Token *token_n = malloc(sizeof(Token));
    if(token_n == NULL){
      return 99;
    }

    if (GET_TOKEN_CHECK_EOF(token_n)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}

    //začátek funkce
    if(TOKEN_TYPE_NEEDED_CHECK(token_n->type, TypeLeftBracket)){
      fction(token_n);

    }

    if (TOKEN_TYPE_NEEDED_CHECK(token_n->type, TypeAssignment)) {

      if (symtab_it_position((char*)token->data, table) == NULL) {
        SYMTAB_ADD_VAR(table, token);

      }



      free(token_n->data);
      value(token_n);

      SYMTAB_ADD_VALUE(table, token, token_n);

    }

  }
  return 0;
}

void body(Token *token){
  command(token);
  while (1) {
    if (!(GET_TOKEN_CHECK_EOF(token))) {
      if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {

        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); return;}
        command(token);
      }
      else{
        return;
      }
    }
    else {
      DEBUG_PRINT("Found EOF, parsing terminated.\n");
      return;
    }
  }
}

// FOR NOW EQUAL TO <prog>
// <prog> -> <body>
// <prog> -> <fction_start> INDENT <body> DEDENT <body>
int prog() {
  Token *token = malloc(sizeof(Token));
  if(token == NULL){
    return 99;
  }

  scanner_init();

  if(GET_TOKEN_CHECK_EOF(token)){
    DEBUG_PRINT("Test file is empty.\n");
    return 1;
  }

  table = malloc(sizeof(hSymtab));
  if(table == NULL){
    return 99;
  }
  symtab_init(table);


  if (TOKEN_TYPE_NEEDED_CHECK(token->type, FCTDEF)) {
    return 1;
  }
  else {
    body(token);
  }


  return 0;
}
