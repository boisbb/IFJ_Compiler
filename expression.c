#include <stdlib.h>
#include <stdio.h>

#include "expression.h"
#define P_SIZE 9

hSymtab *table;
TermStack stack;
IdStack id_stack;
hSymtab_it *variable;
Token *act_tok;
int error;

const char *operNames[];

typedef enum {
  OP_MULTDIV,
  OP_MULTDIVFLOOR,
  OP_PLUSMINUS,
  OP_REL,
  LEFTPAR,
  RIGHTPAR,
  IDENTIFIER,
  DOLLAR,
  ERROR
} PRECED_TABLE_ITEM;


char PRECED_TABLE[P_SIZE][P_SIZE] = {
/*|     |      * /   //   +-   R    (    )   ID    $ */
/*| * / |*/  { '>', '>', '>', '>', '<', '>', '<', '>'},
/*|  // |*/  { '>', '>', '>', '>', '<', '>', '<', '>'}, // Revise, may be incorrect
/*| + - |*/  { '<', '<', '>', '>', '<', '>', '<', '>'},
/*|  R  |*/  { '<', '<', '<', '>', '<', '>', '<', '>'}, // Relation operators
/*|  (  |*/  { '<', '<', '<', '<', '<', 'E', '<', 'N'},
/*|  )  |*/  { '>', '>', '>', '>', '<', '>', 'N', '>'},
/*|  ID |*/  { '>', '>', '>', '>', '<', '>', 'N', '>'},
/*|  $  |*/  { '<', '<', '<', '<', '<', '>', '<', 'N'},
};


PRECED_TABLE_ITEM convert_token_type_to_prec_type(Token *token){
  switch (token->type) {
    case TypeOperatorPlus:
    case TypeOperatorMinus:
      return OP_PLUSMINUS;

    case TypeOperatorMul:
    case TypeOperatorDiv:
      return OP_MULTDIV;

    case TypeEquality:
    case TypeGreater:
    case TypeGreaterEq:
    case TypeLesser:
    case TypeLesserEq:
    case TypeNegation:
    case TypeUnEquality:
      return OP_REL;

    case TypeLeftBracket:
      return LEFTPAR;

    case TypeRightBracket:
      return RIGHTPAR;

    case TypeVariable:
    case TypeInt:
    case TypeString:
    case TypeFloat:
      return IDENTIFIER;

    case TypeKeyword:
    case TypeNewLine:
    case TypeIndent:
    case TypeDedend:
    case TypeColon:
      return DOLLAR;
    default:
      return ERROR;
  }
}

// Processes the token
int expression(Token *act_token, hSymtab_it *p_variable){
  variable = p_variable;
  act_tok = act_token;
  error = expression_eval();
  act_token = act_tok;
  return error;
}


int expression_eval(){
  term_stack_init();
  int token_index = 0;

  while (1) {

    token_index = convert_token_type_to_prec_type(act_tok);
    //DEBUG_PRINT("\nstack: %d token: %d\n\n", stack.top->prec_tab_id, token_index);

    //DEBUG_PRINT("index: %c\n", PRECED_TABLE[stack.top->prec_tab_id][token_index]);

    switch (PRECED_TABLE[stack.top->prec_tab_id][token_index]) {

      // Poping LEFTPAR from stack
      case 'E':
        //DEBUG_PRINT("Deleting: %s", operNames[stack.top->type]);
        s_pop();

        if(get_next_token(act_tok) == EOF) {
          DEBUG_PRINT("Parsing ended: found EOF.\n");
          return EOF;
        }

        break;

      // Pushing on stack
      case '<':
        //DEBUG_PRINT("Pushing: %s\n", operNames[act_tok->type]);
        if (s_push(act_tok) == ERROR_SEMANTIC)
          return ERROR_SEMANTIC;

        if(get_next_token(act_tok) == EOF) {
          DEBUG_PRINT("Parsing ended: found EOF.\n");
          return EOF;
        }

        break;

      // Popping from stack and generating code for operand/operator
      case '>':
        //DEBUG_PRINT("Popping: %s\n", operNames[stack.top->type]);
        if (ready_to_pop() == ERROR_SYNTAX)
          return ERROR_SYNTAX;

        break;
      default:

      if (act_tok->type == TypeNewLine) {
        printf("FOUND NEWLINE\n");
        return NO_ERROR;
      }

        DEBUG_PRINT("wut bruv\n");
        return ERROR_SEMANTIC;
    }


  }
  return NO_ERROR;

}

// Initialization of the term stack
int term_stack_init(){
  if(!(stack.top = malloc(sizeof(TermStackIt)))){ DEBUG_PRINT("ERROR: allocation."); return ERROR_SEMANTIC_RUNTIME;}
  stack.top->next = NULL;
  stack.top->prev = NULL;
  stack.top->tok_cont = NULL;
  stack.top->type = TypeUnspecified;
  stack.top->prec_tab_id = DOLLAR;
}


int s_push(){
  if (act_tok->type == TypeVariable){
    if (((hSymtab_Var*)(symtab_it_position((char*)act_tok->data, table)->data))->defined == false) {
      DEBUG_PRINT("ERROR: variable %s does not exist.\n", (char*)act_tok->data);
      return ERROR_SEMANTIC;
    }
  }

  if (!(stack.top->next = malloc(sizeof(TermStackIt))) || !(stack.top->next->prev = malloc(sizeof(TermStackIt)))) {
    DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
    return ERROR_INTERNAL;
  }

  stack.top->next->prev = stack.top;
  stack.top = stack.top->next;
  stack.top->prec_tab_id = convert_token_type_to_prec_type(act_tok);
  stack.top->type = act_tok->type;
  stack.top->tok_cont = act_tok->data;
  stack.top->next = NULL;

  return NO_ERROR;
}

int s_pop(){
  stack.top = stack.top->prev;
  free(stack.top->next);
  stack.top->next = NULL;
}

int id_stack_init(){
  //if(!(id_stack.top = malloc(sizeof(TermStackIt)))){ DEBUG_PRINT("ERROR: allocation."); return ERROR_SEMANTIC_RUNTIME;}
  id_stack.top = NULL;
}

int id_s_push(TermStackIt *term_item){
  IdStackIt *item = malloc(sizeof(IdStackIt));
  if (!(item = malloc(sizeof(IdStackIt)))) {
    DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
    return ERROR_INTERNAL;
  }

  item->content = term_item->tok_cont;
  item->type = term_item->type;
  item->left = id_stack.top;
  item->right = NULL;
  id_stack.top = item;
}

int id_s_pop(){
  id_stack.top = id_stack.top->left;
  free(id_stack.top->right);
  id_stack.top->right = NULL;
}

int ready_to_pop(){
  switch (stack.top->prec_tab_id) {

    case IDENTIFIER:
      if (id_stack.top == NULL) {
        id_stack_init();
      }

      switch (stack.top->type) {
        case TypeVariable:
          if (!(symtab_it_position((char*)stack.top->tok_cont, table))) {
            DEBUG_PRINT("ERROR: variable %s does not exist.\n", (char*)stack.top->tok_cont);
            return ERROR_SEMANTIC;
          }
          else {
            if (symtab_it_position((char*)stack.top->tok_cont, table)->item_type == IT_VAR){
              id_s_push(stack.top);

            }
            /*

                FUNCTION CALL

            */
          }
          break;
        case TypeFloat:
        case TypeInt:
        case TypeString:

          // GENERATE INSTRUCTION //
          if (stack.top->type == TypeInt)
            printf("\t \tSENT TO GENERATOR: %d\n", *(int*)stack.top->tok_cont);
          else if (stack.top->type == TypeFloat)
            printf("\t \tSENT TO GENERATOR: %f\n", *(double*)stack.top->tok_cont);
          else
            printf("\t \tSENT TO GENERATOR: %s\n", (char*)stack.top->tok_cont);
          ///

          id_s_push(stack.top);
          break;

        default:
          break;
      }
      break;

    case OP_PLUSMINUS:
      if(check_operators_and_operands_syntax(stack.top->type) == ERROR_SYNTAX)
        return ERROR_SYNTAX;

      // GENERATE INSTRUCTION //
      printf("\t \tSENT TO GENERATOR: %c\n", stack.top->type == TypeOperatorPlus ? '+' : '-');
      ///
      break;
    case OP_MULTDIV:
      if(check_operators_and_operands_syntax(stack.top->type) == ERROR_SYNTAX)
        return ERROR_SYNTAX;

      // GENERATE INSTRUCTION //
      printf("\t \tSENT TO GENERATOR: %c\n", stack.top->type == TypeOperatorMul ? '*' : '/');
      ///
      break;

    case OP_REL:
      if(check_operators_and_operands_syntax(stack.top->type) == ERROR_SYNTAX)
        return ERROR_SYNTAX;

      // GENERATE INSTRUCTION //
      printf("\t \tSENT TO GENERATOR: %s\n", operNames[stack.top->type]);
      ///
      break;


    default:
      break;
  }


  s_pop();
  /*
  if (id_stack.top->type == TypeInt)
    printf("\t \tID STACK TOP: %d | STACK TOP TYPE: %d\n", *(int*)id_stack.top->content, stack.top->prec_tab_id);
  else if (id_stack.top->type == TypeFloat)
    printf("\t \tID STACK TOP: %f | STACK TOP TYPE: %d\n", *(double*)id_stack.top->content, stack.top->prec_tab_id);
  else
    printf("\t \tID STACK TOP: %s | STACK TOP TYPE: %d\n", (char*)id_stack.top->content, stack.top->prec_tab_id);
  */
  return NO_ERROR;

}

// IMPLEMENTOVAT OPERATOR  '//'
int check_operators_and_operands_syntax(Type operator){

  if (!(id_stack.top->left)) {
    DEBUG_PRINT("SYNTAX ERROR: two operators.\n");
    return ERROR_SYNTAX;
  }


  IdStackIt l_operand = *id_stack.top->left;
  IdStackIt r_operand = *id_stack.top;
  Type result;

  // When both operands are INT, there is no need for any syntax check
  if (l_operand.type == TypeInt && r_operand.type == TypeInt && operator != TypeOperatorDiv) {

    // If variable is NULL, then the expression is part of if or while statement
    if((!variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      id_s_pop();
      return NO_ERROR;
    }
    else if((variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      DEBUG_PRINT("SYNTAX ERROR: Boolop to be implemented.\n");
      return ERROR_SYNTAX;
    }
    else {
      result = TypeInt;
      symtab_add_var_data(variable, TypeInt);
    }
  }
  else if(l_operand.type == TypeInt && r_operand.type == TypeInt && operator == TypeOperatorDiv){
    result = TypeFloat;
    symtab_add_var_data(variable, TypeFloat);
  } // When both operands are either Float or INT or any combination, there is no need to check syntax
  else if ((l_operand.type == TypeFloat && r_operand.type == TypeInt) || (l_operand.type == TypeInt && r_operand.type == TypeFloat) ||
            (l_operand.type == TypeFloat && r_operand.type == TypeFloat)){

    // If variable is NULL, then the expression is part of if or while statement
    if((!variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      id_s_pop();
      return NO_ERROR;
    }
    else if(variable && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      DEBUG_PRINT("SYNTAX ERROR: Boolop to be implemented.\n");
      return ERROR_SYNTAX;
    }
    else {
      result = TypeFloat;
      symtab_add_var_data(variable, TypeFloat);
    }
  }
  else if (l_operand.type == TypeString && r_operand.type == TypeString){
    if (operator != TypeOperatorPlus) {
      DEBUG_PRINT("SYNTAX ERROR: Incorrect operator or operand.\n");
      return ERROR_SYNTAX;
    }
    result = TypeString;
    symtab_add_var_data(variable, TypeString);
  } // If for TypeUnspecified that comes out of function PARAMETERS
  else if (((l_operand.type == TypeUnspecified) && (r_operand.type == TypeUnspecified || r_operand.type == TypeInt ||
    r_operand.type == TypeFloat || r_operand.type == TypeString)) || ((r_operand.type == TypeUnspecified || r_operand.type == TypeInt ||
    r_operand.type == TypeFloat || r_operand.type == TypeString) && (r_operand.type == TypeUnspecified))) {
    symtab_add_var_data(variable, TypeUnspecified);
  }
  else {
    DEBUG_PRINT("SYNTAX ERROR: Incorrect operator or operand.\n");
    return ERROR_SYNTAX;
  }

  id_s_pop();
  id_stack.top->type = result;

  return NO_ERROR;

}
