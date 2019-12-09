#include <stdlib.h>
#include <stdio.h>

#include "expression.h"
#include "generator.h"
#define P_SIZE 9
#define BRACKET_RETURN 800


hSymtab *s_table;
TermStack expr_stack;
IdStack id_stack;
hSymtab_it *variable;
Token act_tok;
Token pre_tok;
int error;
int id_stack_cnt = 0;

Type param_type;

const char *operNames_[] = {"+", "-", "*", "/", "//", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "new line", "keyword", "variable", "string", "documentary string", "int", "float", "indent", "dedent", "None", "pass", "unspecified", "func", "undefined"};


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
    case TypeOperatorFloorDiv:
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
    case TypeKeywordNone:
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
int expression(Token *pre_token, Token *act_token, hSymtab_it *p_variable, hSymtab *actual_table){
  variable = p_variable;
  act_tok = *act_token;
  s_table = actual_table;

  if (pre_token) {
    pre_tok = *pre_token;
  }

  if ((convert_token_type_to_prec_type(&pre_tok) == IDENTIFIER) && convert_token_type_to_prec_type(&act_tok) != OP_PLUSMINUS &&
      convert_token_type_to_prec_type(&act_tok) != OP_MULTDIV && convert_token_type_to_prec_type(&act_tok) != OP_REL && convert_token_type_to_prec_type(&act_tok) != OP_MULTDIVFLOOR) {
    return ERROR_SYNTAX;
  }

  //////fprintf(stderr, "%s\n", (char*)act_tok.data); SEG FAULT
  error = expression_eval(0, pre_token != NULL);

  if (error) {
    return error;
  }

  //// PLYNE Z OPRAVY, ZAKOMENTOVANA CAST V ready_to_pop VICKRAT OKONTROLOVAT TODO
  if (variable) {
    if (variable->item_type == IT_VAR){
      ((hSymtab_Var*)variable->data)->type = id_stack.top->type;
    }
  }

  free(expr_stack.top);
  free(id_stack.top);
  id_stack.top = NULL;

  *act_token = act_tok;
  return error;
}


int expression_eval(int fction_switch, int pre_token_switch){
  if (fction_switch == 0) {
    term_stack_init();
  }

  ////
  if (pre_token_switch) {
    TermStackIt tmp_s_it;
    if (pre_tok.type == TypeVariable) {
      if (symtab_it_position((char*)pre_tok.data, s_table)) {


        //    TODO WOT
        //GENERATE CODE FOR ASSIGN TOKEN
        ////fprintf(stderr,"\t \tSENT TO GENERATOR: %s\n", (char*)pre_tok.data);


      }
      else {
        ////DEBUG_PRINT("Variable %s does not exist\n", (char*)pre_tok.data);
        return ERROR_SEMANTIC;
      }
    }
    tmp_s_it.tok_cont = pre_tok.data;
    tmp_s_it.type = pre_tok.type;
    error = id_s_push(&tmp_s_it);
    if (error != NO_ERROR) {
      return error;
    }
  }
  ////

  int token_index = 0;
  int par_cnt = 1;

  while (1) {


    // Pro rozsireni, zatim se nepouziva ////////////////
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
      ////DEBUG_PRINT("Syntax error: comma is not supported\n");
      return ERROR_SYNTAX;
    }
    /////////////////////////////////////////////////////


    token_index = convert_token_type_to_prec_type(&act_tok);
    if (token_index == ERROR) {
      return ERROR_SYNTAX;
    }

    switch (PRECED_TABLE[expr_stack.top->prec_tab_id][token_index]) {

      // Poping LEFTPAR from stack
      case 'E':
        //////DEBUG_PRINT("Deleting: %s", operNames_[stack.top->type]);
        s_pop();

        if(get_next_token(&act_tok) == EOF) {
          ////DEBUG_PRINT("Parsing ended: found EOF.\n");
          return NO_ERROR;
        }

        break;

      // Pushing on stack
      case '<':
        ////DEBUG_PRINT("Pushing: %s\n", operNames_[act_tok.type]);
        if (act_tok.type == TypeVariable) {
          if (!symtab_it_position((char*)act_tok.data, s_table)) {

            if (get_next_token(&act_tok)) {
              return ERROR_SEMANTIC;
            }
            if (act_tok.type == TypeLeftBracket) {
              return BRACKET_RETURN;
            }
            else {
              return ERROR_SEMANTIC;
            }
          }
        }

        if ((error = s_push(&act_tok)) != NO_ERROR){
          ////fprintf(stderr, "%s\n", (char*)act_tok.data);
          return error;
        }


        if(get_next_token(&act_tok) == EOF) {
          ////DEBUG_PRINT("Parsing ended: found EOF.\n");
          return ERROR_SYNTAX;
        }

        break;

      // Popping from stack and generating code for operand/operator
      case '>':
        ////DEBUG_PRINT("Popping: %s\n", operNames_[expr_stack.top->type]);
        if ((error = ready_to_pop(fction_switch)) != NO_ERROR)
          return error;

        break;
      default:

        if ((act_tok.type == TypeNewLine || act_tok.type == TypeColon) && expr_stack.top->type != TypeLeftBracket ) {
          ////fprintf(stderr,"FOUND NEWLINE OR COLON\n");
          return NO_ERROR;
        }

        ////DEBUG_PRINT("SYNTAX ERROR: Wrong input.\n");
        return ERROR_SYNTAX;
    }


  }
  return NO_ERROR;

}

// Initialization of the term stack
int term_stack_init(){
  if(!(expr_stack.top = malloc(sizeof(TermStackIt)))){ ////DEBUG_PRINT("ERROR: allocation.");
    return ERROR_INTERNAL;
  }
  expr_stack.top->next = NULL;
  expr_stack.top->prev = NULL;
  expr_stack.top->tok_cont = NULL;
  expr_stack.top->type = TypeUnspecified;
  expr_stack.top->prec_tab_id = DOLLAR;
  return NO_ERROR;
}


int s_push(){

  if (act_tok.type == TypeVariable){
    if ((error = is_item_var_defined((char*)act_tok.data, s_table)) != NO_ERROR) {
      return error;
    }
  }
                                                              /// REDUNDANT ALLOCATION //
  if (!(expr_stack.top->next = malloc(sizeof(TermStackIt))) /*|| !(stack.top->next->prev = malloc(sizeof(TermStackIt)))*/) {
    ////DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
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

void s_pop(){
  expr_stack.top = expr_stack.top->prev;
  free(expr_stack.top->next);
  expr_stack.top->next = NULL;
}

void s_free(){
}

void id_stack_init(){
  id_stack.top = NULL;
}

int id_s_push(TermStackIt *term_item){
  id_stack_cnt += 1;
  Type type;



  if (term_item->type == TypeVariable){
    type = ((hSymtab_Var*)(symtab_it_position((char*)term_item->tok_cont, s_table)->data))->type;
    ////DEBUG_PRINT("TYPE IN id_s_push: %s\n", operNames_[type]);
  }
  else {
    type = term_item->type;
  }

  if (!id_stack.top) {
    if (!(id_stack.top = malloc(sizeof(IdStackIt)))) {
      ////DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
      return ERROR_INTERNAL;
    }
    id_stack.top->right = NULL;
    id_stack.top->left = NULL;
    id_stack.top->type = type;
    id_stack.top->content = term_item->tok_cont;
    return NO_ERROR;
  }

  if (!(id_stack.top->right = malloc(sizeof(IdStackIt)))) {
    ////DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
    return ERROR_INTERNAL;
  }

  id_stack.top->right->left = id_stack.top;
  id_stack.top = id_stack.top->right;
  id_stack.top->type = type;
  id_stack.top->content = term_item->tok_cont;
  id_stack.top->right = NULL;
  return NO_ERROR;
}

void id_s_pop(){
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
            ////DEBUG_PRINT("ERROR: variable %s does not exist.\n", (char*)expr_stack.top->tok_cont);
            return ERROR_SEMANTIC;
          }
          else {
            if (symtab_it_position((char*)expr_stack.top->tok_cont, s_table)->item_type == IT_VAR){
              id_s_push(expr_stack.top);
              ////DEBUG_PRINT("Pushed to id stack: %s %d\n", (char*)id_stack.top->content, id_stack.top->type);

              if (fction_switch != 1){
              }
              else {
                param_type = id_stack.top->type;
              }

              ////fprintf(stderr,"\t \tSENT TO GENERATOR: %s\n", (char*)expr_stack.top->tok_cont);
              bool scope = ((hSymtab_Var*)symtab_it_position((char*)expr_stack.top->tok_cont, s_table)->data)->global;
              generate_push_var((char*)expr_stack.top->tok_cont, scope);
            }
            /*
                FUNCTION CALL
            */
          }
          break;

        case TypeFloat:
        case TypeInt:
        case TypeString:
        case TypeKeywordNone:

          generate_push_data(expr_stack.top->type, expr_stack.top->tok_cont);

          id_s_push(expr_stack.top);
          //////DEBUG_PRINT("Pushed to id stack: %f\n", *(double*)expr_stack.top->tok_cont);

          if (fction_switch != 1){
          }
          else {
            param_type = id_stack.top->type;
          }

          break;

        default:
          return ERROR_SYNTAX;
      }
      break;

    case OP_PLUSMINUS:
      if((error = check_operators_and_operands_syntax(expr_stack.top->type, fction_switch)) != NO_ERROR){
        ////fprintf(stderr, "%d\n", error);
        return error;
      }

      // GENERATE INSTRUCTION //
      ////fprintf(stderr,"\t \tSENT TO GENERATOR: %c\n", expr_stack.top->type == TypeOperatorPlus ? '+' : '-');
      ///
      break;
    case OP_MULTDIV:
      if((error = check_operators_and_operands_syntax(expr_stack.top->type, fction_switch)) != NO_ERROR)
        return error;

      // GENERATE INSTRUCTION //
      ////fprintf(stderr,"\t \tSENT TO GENERATOR: %c\n", expr_stack.top->type == TypeOperatorMul ? '*' : '/');
      ///
      break;

    case OP_REL:
      if((error = check_operators_and_operands_syntax(expr_stack.top->type, fction_switch)) != NO_ERROR)
        return error;

      // GENERATE INSTRUCTION //
      ////fprintf(stderr,"\t \tSENT TO GENERATOR: %s\n", operNames_[expr_stack.top->type]);
      ///
      break;


    default:
      return ERROR_SYNTAX;
      break;
  }



  s_pop();
  return NO_ERROR;

}


int check_operators_and_operands_syntax(Type operator, int fction_switch){

  if (!(id_stack.top->left)) {
    ////DEBUG_PRINT("SYNTAX ERROR: two operators.\n");
    return ERROR_SYNTAX;
  }
  ////fprintf(stderr, "a\n");


  IdStackIt l_operand = *id_stack.top->left;
  IdStackIt r_operand = *id_stack.top;
  Type result;

  ////DEBUG_PRINT("L: %s O: %s R: %s", operNames_[l_operand.type], operNames_[operator], operNames_[r_operand.type]);

  if (r_operand.type == TypeKeywordNone || l_operand.type == TypeKeywordNone) {
    if (operator == TypeEquality || operator == TypeUnEquality) {
      id_s_pop();
      generate_operation(operator); //generator
      return NO_ERROR;
    }
    else {
      return ERROR_SEMANTIC_RUNTIME;
    }
  }


  if ((operator == TypeOperatorDiv || operator == TypeOperatorFloorDiv) && r_operand.type == TypeInt) {
    if (*(int*)r_operand.content == 0) {
      ////fprintf(stderr, "a\n");
      return ERROR_DIV_BY_ZERO;
    }
  }

  if((!variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
      operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){

    ////fprintf(stderr, "CHECK\n");

    if (l_operand.type == TypeUnspecified){

      generate_operation_unspecified(operator);
      return NO_ERROR;

    }
    else if (r_operand.type == TypeUnspecified){
      generate_operation_unspecified(operator);
      // TODO

      return NO_ERROR;
    }
    else if(l_operand.type == TypeUnspecified && r_operand.type == TypeUnspecified)
    {
        generate_operation_unspecified(operator);
        return NO_ERROR;
    }
    else if (l_operand.type == r_operand.type) {
       id_s_pop();
       generate_operation(operator); //generator
       return NO_ERROR;
    }
    else if ((l_operand.type == TypeInt && r_operand.type == TypeFloat)) {
      generate_operation_retype_sec_int2float();
      generate_operation(operator);
      return NO_ERROR;
    }
    else if ((l_operand.type == TypeFloat && r_operand.type == TypeInt)) {
      generate_operation_retype_first_int2float();
      generate_operation(operator);
      return NO_ERROR;
    }
    else {
      return ERROR_SEMANTIC_RUNTIME;
    }
  }
  else if((variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
      operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
    ////DEBUG_PRINT("SYNTAX ERROR: Boolop to be implemented.\n");
    return ERROR_SEMANTIC_RUNTIME;
  }
  else if (l_operand.type == TypeInt && r_operand.type == TypeInt && operator != TypeOperatorDiv) {
    // If variable is NULL, then the expression is part of if or while statement
    if((!variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      id_s_pop();
      generate_operation(operator); //generator
      return NO_ERROR;
    }
    else if((variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      ////DEBUG_PRINT("SYNTAX ERROR: Boolop to be implemented.\n");
      return ERROR_SEMANTIC_RUNTIME;
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
    generate_operation(operator); //generator

  }
  else if(l_operand.type == TypeInt && r_operand.type == TypeInt && operator == TypeOperatorDiv){
    result = TypeFloat;

    generate_operation_retype_first_int2float();
    generate_operation_retype_sec_int2float();

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
    generate_operation(operator); //generator
  } // For TypeOperatorFloorDiv TODO
  else if( operator == TypeOperatorFloorDiv && (l_operand.type == TypeInt && r_operand.type == TypeInt)) {
    result = TypeInt;

    // TODO GENERATE CODE FOR TypeOperatorFloorDiv


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
  }
  else if (((l_operand.type == TypeFloat && r_operand.type == TypeInt) || (l_operand.type == TypeInt && r_operand.type == TypeFloat) ||
            (l_operand.type == TypeFloat && r_operand.type == TypeFloat)) && operator != TypeOperatorFloorDiv){

    if(l_operand.type == TypeInt)
        generate_operation_retype_sec_int2float();
    else if(r_operand.type == TypeInt)
        generate_operation_retype_first_int2float();

    // If variable is NULL, then the expression is part of if or while statement
    if((!variable) && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      generate_operation(operator); //generator
      id_s_pop();
      return NO_ERROR;
    }
    else if(variable && (operator == TypeEquality || operator == TypeUnEquality || operator == TypeGreater ||
        operator == TypeGreaterEq || operator == TypeLesser || operator == TypeLesserEq || operator == TypeNegation)){
      ////DEBUG_PRINT("SYNTAX ERROR: Boolop to be implemented.\n");
      return ERROR_SEMANTIC_RUNTIME;
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
    generate_operation(operator); //generator
  }
  else if (l_operand.type == TypeString && r_operand.type == TypeString && operator != TypeOperatorFloorDiv){
    if (operator != TypeOperatorPlus) {
      ////DEBUG_PRINT("SYNTAX ERROR: Incorrect operator or operand.\n");
      return ERROR_SEMANTIC_RUNTIME;
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
    generate_operation_concat();
  } // If for TypeUnspecified that comes out of function PARAMETERS
  else if (((l_operand.type == TypeUnspecified) && (r_operand.type == TypeInt ||
    r_operand.type == TypeFloat || r_operand.type == TypeString))
    ||
    ((l_operand.type == TypeInt ||
    l_operand.type == TypeFloat || l_operand.type == TypeString) && (r_operand.type == TypeUnspecified))) {
    result = r_operand.type;
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
    //generate_operation(operator); //generator
    generate_operation_unspecified(operator); //generator TODO
  }
  else if(r_operand.type == TypeUnspecified && l_operand.type == TypeUnspecified)
  {
      result = TypeUnspecified;
      generate_operation_unspecified(operator); //generator
  }
  else {
    ////DEBUG_PRINT("SYNTAX ERROR: Incorrect operator or operand.\n");
    return ERROR_SEMANTIC_RUNTIME;
  }

  id_s_pop();
  id_stack.top->type = result;


  return NO_ERROR;

}
