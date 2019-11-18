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

void assignment(Token *token, Token *token_next){
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}


  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt)
      || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeString)
      || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeFloat)
      || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {

        // TADY SE BUDE TVORIT INSTRUKCE

        return;
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
  //potřebuju nový token, teď mám načtenou levou závorku
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
  while(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)){
    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
      printf("var\n");
      //zapsat parametry do tabulky bez typu

    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeComma)){
      printf("comma\n");
    }
    else{
      return 1;
    }
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
  }
  return 0;
}

int fction_start(Token *token){
  //return 1 = chyba
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  //zacatek funkce
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
    Token *fction_name = token;

    //kontrola, jeslti následuje závorka a jestli není EOF
    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeLeftBracket)){
      //kontrola jeslti funcke s tímto jménem není už v tabulce
      if(symtab_it_position((char*)token->data, table) == NULL){
        //printf("dobrý\n");
        fction_name->type = TypeFunc;
        symtab_add_it(table, fction_name); //přidám do tabulky název funcke
        if(fction_params(token) != 0){
          return 1;
        }
        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
        if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)){
          printf("spravně\n");
        }
        //fction_body();

      }
      else{
        return 1; //funcke už v tabulce je
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




  if (strcmp((char*)token->data, "def") == 0) {
    fction_start(token); //kontrolovat, co to vrátilo, kvůli returnům
    return 0;
  }
  else {
    body(token);
  }

  return 0;
}
