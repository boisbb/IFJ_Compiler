#include <stdlib.h>
#include <stdio.h>

#include "expression.h"
#define P_SIZE 9

hSymtab *s_table;
TermStack expr_stack;
IdStack id_stack;
hSymtab_it *variable;
Token act_tok;
int error;
int id_stack_cnt = 0;

Type param_type;


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
    case TypeComma:
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
int expression(Token *act_token, hSymtab_it *p_variable, hSymtab *actual_table){
  variable = p_variable;
  act_tok = *act_token;
  s_table = actual_table;
  error = expression_eval(0);

  free(expr_stack.top);
  free(id_stack.top);
  id_stack.top = NULL;

  *act_token = act_tok;
  return error;
}


int expression_eval(int fction_switch){
  if (fction_switch == 0) {
    term_stack_init();
  }
  int token_index = 0;
  int par_cnt = 1;

  while (1) {

    if (fction_switch == 1) {
      if ((act_tok.type == TypeComma || act_tok.type == TypeRightBracket) && expr_stack.top->type == TypeLeftBracket) {
        par_cnt--;
        if (par_cnt == 0) {
          return NO_ERROR;
        }
        else {
          return ERROR_SYNTAX;
        }
      }
    }
    else if(fction_switch != 1 && act_tok.type == TypeComma){
      DEBUG_PRINT("Syntax error: comma is not supported\n");
      return ERROR_SYNTAX;
    }

    token_index = convert_token_type_to_prec_type(&act_tok);

    switch (PRECED_TABLE[expr_stack.top->prec_tab_id][token_index]) {

      // Poping LEFTPAR from stack
      case 'E':
        //DEBUG_PRINT("Deleting: %s", operNames[stack.top->type]);
        s_pop();

        if(get_next_token(&act_tok) == EOF) {
          DEBUG_PRINT("Parsing ended: found EOF.\n");
          return EOF;
        }

        break;

      // Pushing on stack
      case '<':
        //DEBUG_PRINT("Pushing: %s\n", operNames[act_tok.type]);

        if (act_tok.type == TypeVariable) {
          hSymtab_it *tmp_sym_it = symtab_it_position((char*)act_tok.data, s_table);
          if (tmp_sym_it) {
            if (tmp_sym_it->item_type == IT_FUNC) {
              error = realize_function_call((hSymtab_Func*)tmp_sym_it->data);
              if (error != NO_ERROR) {
                return error;
              }
            }
          }
        }

        if (s_push(&act_tok) == ERROR_SEMANTIC)
          return ERROR_SEMANTIC;

        if(get_next_token(&act_tok) == EOF) {
          DEBUG_PRINT("Parsing ended: found EOF.\n");
          return EOF;
        }

        break;

      // Popping from stack and generating code for operand/operator
      case '>':
        //DEBUG_PRINT("Popping: %s\n", operNames[expr_stack.top->type]);
        if (ready_to_pop(fction_switch) == ERROR_SYNTAX)
          return ERROR_SYNTAX;

        break;
      default:

      if (act_tok.type == TypeNewLine) {
        printf("FOUND NEWLINE\n");
        return NO_ERROR;
      }

        DEBUG_PRINT("SYNTAX ERROR: Wrong input.\n");
        return ERROR_SYNTAX;
    }


  }
  return NO_ERROR;

}

// Initialization of the term stack
int term_stack_init(){
  if(!(expr_stack.top = malloc(sizeof(TermStackIt)))){ DEBUG_PRINT("ERROR: allocation."); return ERROR_SEMANTIC_RUNTIME;}
  expr_stack.top->next = NULL;
  expr_stack.top->prev = NULL;
  expr_stack.top->tok_cont = NULL;
  expr_stack.top->type = TypeUnspecified;
  expr_stack.top->prec_tab_id = DOLLAR;
}


int s_push(){

  if (act_tok.type == TypeVariable){
    if ((error = is_item_var_defined((char*)act_tok.data, s_table)) != NO_ERROR) {
      return error;
    }
  }
                                                              /// REDUNDANT ALLOCATION //
  if (!(expr_stack.top->next = malloc(sizeof(TermStackIt))) /*|| !(stack.top->next->prev = malloc(sizeof(TermStackIt)))*/) {
    DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
    return ERROR_INTERNAL;
  }

  expr_stack.top->next->prev = expr_stack.top;
  expr_stack.top = expr_stack.top->next;
  expr_stack.top->prec_tab_id = convert_token_type_to_prec_type(&act_tok);
  expr_stack.top->type = act_tok.type;
  expr_stack.top->tok_cont = act_tok.data;
  expr_stack.top->next = NULL;

  return NO_ERROR;
}

int s_pop(){
  expr_stack.top = expr_stack.top->prev;
  free(expr_stack.top->next);
  expr_stack.top->next = NULL;
}

int s_free(){

  //s_pop();

  if (expr_stack.top->next == NULL) {
    printf("NULLIOBROLIO\n");
    exit(1);
  }
}

int id_stack_init(){
  id_stack.top = NULL;
}

int id_s_push(TermStackIt *term_item){
  id_stack_cnt += 1;
  Type type;

  if (term_item->type == TypeVariable){
    type = ((hSymtab_Var*)(symtab_it_position((char*)term_item->tok_cont, s_table)->data))->type;
  }
  else {
    type = term_item->type;
  }

  if (!id_stack.top) {
    if (!(id_stack.top = malloc(sizeof(IdStackIt)))) {
      DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
      return ERROR_INTERNAL;
    }
    id_stack.top->right = NULL;
    id_stack.top->left = NULL;
    id_stack.top->type = type;
    id_stack.top->content = term_item->tok_cont;
    return NO_ERROR;
  }

  if (!(id_stack.top->right = malloc(sizeof(IdStackIt)))) {
    DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
    return ERROR_INTERNAL;
  }

  id_stack.top->right->left = id_stack.top;
  id_stack.top = id_stack.top->right;
  id_stack.top->type = type;
  id_stack.top->content = term_item->tok_cont;
  id_stack.top->right = NULL;
}

int id_s_pop(){
  id_stack_cnt -= 1;
  id_stack.top = id_stack.top->left;
  free(id_stack.top->right);
}

int ready_to_pop(int fction_switch){
  switch (expr_stack.top->prec_tab_id) {

    case IDENTIFIER:

      if (id_stack.top == NULL) {
        id_stack_init();
      }

      switch (expr_stack.top->type) {
        case TypeVariable:
          if (!(symtab_it_position((char*)expr_stack.top->tok_cont, s_table))) {
            DEBUG_PRINT("ERROR: variable %s does not exist.\n", (char*)expr_stack.top->tok_cont);
            return ERROR_SEMANTIC;
          }
          else {
            if (symtab_it_position((char*)expr_stack.top->tok_cont, s_table)->item_type == IT_VAR){
              id_s_push(expr_stack.top);

              if (fction_switch != 1){
                // Maybe change //
                if (variable) {
                  if (variable->item_type == IT_VAR){
                    ((hSymtab_Var*)variable->data)->type = id_stack.top->type;
                  }
                  else if (variable->item_type == IT_FUNC){
                    ((hSymtab_Func*)variable->data)->return_type = id_stack.top->type;
                  }
                }
              }
              else {
                param_type = id_stack.top->type;
              }

              printf("\t \tSENT TO GENERATOR: %s\n", (char*)expr_stack.top->tok_cont);

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
          if (expr_stack.top->type == TypeInt)
            printf("\t \tSENT TO GENERATOR: %d\n", *(int*)expr_stack.top->tok_cont);
          else if (expr_stack.top->type == TypeFloat)
            printf("\t \tSENT TO GENERATOR: %f\n", *(double*)expr_stack.top->tok_cont);
          else
            printf("\t \tSENT TO GENERATOR: %s\n", (char*)expr_stack.top->tok_cont);
          ///

          id_s_push(expr_stack.top);

          if (fction_switch != 1){
            // Maybe change //
            if (variable) {
              if (variable->item_type == IT_VAR){
                ((hSymtab_Var*)variable->data)->type = id_stack.top->type;
              }
              else if (variable->item_type == IT_FUNC){
                ((hSymtab_Func*)variable->data)->return_type = id_stack.top->type;
              }
            }
          }
          else {
            param_type = id_stack.top->type;
          }

          break;

        default:
          break;
      }
      break;

    case OP_PLUSMINUS:
      if(check_operators_and_operands_syntax(expr_stack.top->type, fction_switch) == ERROR_SYNTAX)
        return ERROR_SYNTAX;

      // GENERATE INSTRUCTION //
      printf("\t \tSENT TO GENERATOR: %c\n", expr_stack.top->type == TypeOperatorPlus ? '+' : '-');
      ///
      break;
    case OP_MULTDIV:
      if(check_operators_and_operands_syntax(expr_stack.top->type, fction_switch) == ERROR_SYNTAX)
        return ERROR_SYNTAX;

      // GENERATE INSTRUCTION //
      printf("\t \tSENT TO GENERATOR: %c\n", expr_stack.top->type == TypeOperatorMul ? '*' : '/');
      ///
      break;

    case OP_REL:
      if(check_operators_and_operands_syntax(expr_stack.top->type, fction_switch) == ERROR_SYNTAX)
        return ERROR_SYNTAX;

      // GENERATE INSTRUCTION //
      printf("\t \tSENT TO GENERATOR: %s\n", operNames[expr_stack.top->type]);
      ///
      break;


    default:
      break;
  }


  s_pop();
  return NO_ERROR;

}

int realize_function_call(hSymtab_Func* func_data){
  int param_cnt = symtab_num_of_fction_params(func_data);
  hSymtab_Func_Param *act_parameters = func_data->params;

  if (variable != NULL) {
    if (func_data->return_type == TypeUndefined) {
      DEBUG_PRINT("Syntax error: function has no return value.\n");
      return ERROR_SYNTAX;
    }
  }

  if (((hSymtab_Var*)variable->data)->type == TypeUnspecified) {
    ((hSymtab_Var*)variable->data)->type = func_data->return_type;
  }


  // Push fction name
  s_push(&act_tok);

  printf("\t \tSENT TO GENERATOR: %s\n", (char*)expr_stack.top->tok_cont);


  if(get_next_token(&act_tok) == EOF) {
    DEBUG_PRINT("Parsing ended: found EOF.\n");
    return EOF;
  }

  if (act_tok.type != TypeLeftBracket) {
    DEBUG_PRINT("SYNTAX ERROR: Expected Left Parentheses.\n");
    return ERROR_SYNTAX;
  }

  s_push(&act_tok);

  if (!act_parameters && param_cnt == 0) {
    if(get_next_token(&act_tok) == EOF) {
      DEBUG_PRINT("Found EOF.\n");
      return EOF;
    }

    if (act_tok.type != TypeRightBracket) {
      DEBUG_PRINT("Syntax error: incorrect parameters.\n");
      return ERROR_SYNTAX;
    }
    else{
      return NO_ERROR;
    }
  }


  while(1) {


    if(get_next_token(&act_tok) == EOF) {
      DEBUG_PRINT("Found EOF.\n");
      return EOF;
    }


    if (convert_token_type_to_prec_type(&act_tok) != IDENTIFIER && act_tok.type != TypeRightBracket) {
      DEBUG_PRINT("SYNTAX ERROR: Expected function parameter or right Parentheses.\n");
      return ERROR_SYNTAX;
    }
    else if (convert_token_type_to_prec_type(&act_tok) == IDENTIFIER && param_cnt != 0){

      expression_eval(REALIZE_FUNC);
      param_cnt--;


      if (act_parameters->param_type != TypeUnspecified && param_type != TypeUnspecified) {
        if (act_parameters->param_type != param_type) {
          DEBUG_PRINT("Syntax error: Incorrect parameter type.\n");
          return ERROR_SYNTAX;
        }
      }

      if (act_tok.type == TypeRightBracket) {
        if (param_cnt == 0) {

          // GENEROVANI KODU MOZNA AZ TADY

          s_pop();
          expr_stack.top->type = func_data->return_type;
          id_s_push(expr_stack.top);
          s_pop();
          return NO_ERROR;
        }
        else{
          DEBUG_PRINT("Semantioc error: Incorrect number of parameters.\n");
          return ERROR_SEMANTIC;
        }
      }

      if (act_tok.type == TypeComma) {
        if (param_cnt == 0) {
          DEBUG_PRINT("Syntax error: Too many parameters\n");
          return ERROR_SYNTAX;
        }
      }

      act_parameters = act_parameters->next;

    }
    else if (convert_token_type_to_prec_type(&act_tok) == IDENTIFIER && param_cnt == 0){
      DEBUG_PRINT("SYNTAX ERROR: Function recieved too many parameters.\n");
      return ERROR_SEMANTIC;
    }
    else if (act_tok.type == TypeRightBracket && param_cnt != 0) {
      DEBUG_PRINT("SYNTAX ERROR: Function did not recieve needed parameters.\n");
      return ERROR_SEMANTIC;
    }
    else if(act_tok.type == TypeRightBracket && param_cnt == 0){
      break;
    }
    else {
      DEBUG_PRINT("SYNTAX ERROR: Function did not recieve comma, parentheses or parameter.\n");
      return ERROR_SEMANTIC;
    }

  }

}

// IMPLEMENTOVAT OPERATOR  '//'
int check_operators_and_operands_syntax(Type operator, int fction_switch){

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

      if (fction_switch != 1){
        if (variable) {
          if (variable->item_type == IT_VAR){
            symtab_add_var_data(variable, TypeInt);
          }
          else if (variable->item_type == IT_FUNC){
            ((hSymtab_Func*)variable->data)->return_type = TypeInt;
          }
        }
      }
      else {
        param_type = TypeInt;
      }

    }
  }
  else if(l_operand.type == TypeInt && r_operand.type == TypeInt && operator == TypeOperatorDiv){
    result = TypeFloat;

    if (fction_switch != 1){
      if (variable) {
        if (variable->item_type == IT_VAR){
          symtab_add_var_data(variable, TypeFloat);
        }
        else if (variable->item_type == IT_FUNC){
          ((hSymtab_Func*)variable->data)->return_type = TypeFloat;
        }
      }
    }
    else {
      param_type = TypeFloat;
    }

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

      if (fction_switch != 1) {
        if (variable) {
          if (variable->item_type == IT_VAR){
            symtab_add_var_data(variable, TypeFloat);
          }
          else if (variable->item_type == IT_FUNC){
            ((hSymtab_Func*)variable->data)->return_type = TypeFloat;
          }
        }
      }
      else {
        param_type = TypeFloat;
      }

    }
  }
  else if (l_operand.type == TypeString && r_operand.type == TypeString){
    if (operator != TypeOperatorPlus) {
      DEBUG_PRINT("SYNTAX ERROR: Incorrect operator or operand.\n");
      return ERROR_SYNTAX;
    }
    result = TypeString;
    if (fction_switch != 1) {
      if (variable) {
        if (variable->item_type == IT_VAR){
          symtab_add_var_data(variable, TypeString);
        }
        else if (variable->item_type == IT_FUNC){
          ((hSymtab_Func*)variable->data)->return_type = TypeString;
        }
      }
    }
    else {
      param_type = TypeString;
    }

  } // If for TypeUnspecified that comes out of function PARAMETERS
  else if (((l_operand.type == TypeUnspecified) && (r_operand.type == TypeUnspecified || r_operand.type == TypeInt ||
    r_operand.type == TypeFloat || r_operand.type == TypeString)) || ((r_operand.type == TypeUnspecified || r_operand.type == TypeInt ||
    r_operand.type == TypeFloat || r_operand.type == TypeString) && (r_operand.type == TypeUnspecified))) {

    if (fction_switch != 1){
      if (variable) {
        if (variable->item_type == IT_VAR){
          symtab_add_var_data(variable, TypeUnspecified);
        }
        else if (variable->item_type == IT_FUNC){
          ((hSymtab_Func*)variable->data)->return_type = TypeUnspecified;
        }
      }
    }
    else {
      param_type = TypeUnspecified;
    }

  }
  else {
    DEBUG_PRINT("SYNTAX ERROR: Incorrect operator or operand.\n");
    return ERROR_SYNTAX;
  }

  id_s_pop();
  id_stack.top->type = result;

  return NO_ERROR;

}
