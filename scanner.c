#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "strings.h"

/////// CONSTANTS ////


/////// STATES ///////
//----
#define TEST_STATE 258
//----
#define STATE_INITIAL 259
#define STATE_EQUALS 260
#define STATE_OPERATOR_TWO_CHARS 261
#define STATE_OPERATOR_ONE_CHAR 262
#define STATE_VARIABLE 263
/////////////////////

///////////////////// - only testing, will be moved to another module afterwards
#define ERROR 0
#define SUCCESS 1

//// ENUM OF TYPES ///
typedef enum type {
  operator,
  keyword,
  variable
} type;
/////////////////////

// KEYWORDS ARRAY ///
const char *keywords[] = {"def", "else", "if", "None", "pass", "return", "while"};
/////////////////////

/// JUST FOR TESTING //
const char *opNames[] = {"operator", "keyword", "variable"};
//////////////////////

// TOKEN STRUCTURE ///
typedef struct token Token;
struct token{
  type token_type;
  char *string;
};
////////////////////


// In actuality, there will be another condition, that the cycle will continue only if parser asks for token,
// the function will cycle, but won't do anything
// In the future, this will be function named get_next_token
int main(){
  int c = 0;
  int break_while = true;
  long state = 0;
  char c_prev;
  Token *token = malloc(sizeof(Token));
  String *str = malloc(sizeof(String));
  int space_flag = 0;

  while((c = fgetc(stdin)) != EOF){
    if (c == EOF) {
      printf("EOF\n");
    }
    break_while = true;
    state = STATE_INITIAL;

    space_flag = 0;


    while(break_while){
      switch(state){

        // STATE_INTIAL decides which will be the next case
        case STATE_INITIAL:
          if (c == '+' || c == '-' || c == '/' || c == '*') {
            state = STATE_OPERATOR_ONE_CHAR;
            break;
          }
          else if(isalpha(c)){
            if(str_init(str) == ERROR){
              printf("chyba 1\n");
              exit(0);
            }
            state = STATE_VARIABLE;
            break;

          }
          else if(c == '='){
            state = STATE_EQUALS;
            break;
          }
          else if(c == ' '){
            state = STATE_INITIAL;

            // Just so it doesnt print spaces
            space_flag = 1;
            //

            break_while = 0;
            break;
          }
          else{
            return 0;
          }
          break;

        // No need to further check any char, because the statement doesn't have to be spaced
        case STATE_OPERATOR_ONE_CHAR:
          token->token_type = operator;
          if ((token->string = malloc(sizeof(char))) == NULL) {
            printf("Error\n");
            return 1;
          }
          token->string[0] = c;
          break_while = false;
          break;

        // Can be merged with the operators? Does the same thing
        case STATE_EQUALS:
          token->token_type = operator;
          if ((token->string = malloc(sizeof(char))) == NULL) {
            printf("Error\n");
            return 1;
          }
          token->string[0] = c;
          break_while = false;
          break;

        // Adds char to dynamic string while c is alphanumeric, if it is not anymore
        // shoves the last char back into stdin and proceeds
        case STATE_VARIABLE:
          if (str_add(str, c) == ERROR) {
            printf("Error in str_add\n");
          }

          while ((c = fgetc(stdin)) != EOF && isalnum(c)) {
            if (str_add(str, c) == ERROR) {
              printf("Error in str_add\n");
            }
          }
          ungetc(c, stdin);
          token->token_type = variable;
          if ((token->string = malloc(sizeof(char) * str->asize)) == NULL) {
            printf("Error\n");
            return 1;
          }
          strcpy(token->string, str->contents);
          break_while = false;
          break;
      }
    }

    // TESTING PURPOSES ONLY
    if (space_flag == 0) {
      printf("Token type: %s | Token string: %s \n", opNames[token->token_type], token->string);
    }
  }
  return 0;
}
