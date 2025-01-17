/* parser.c
// název projektu: Implementace překladače imperativního jazyka IFJ19
// tým:            040
// varianta:       II (implementace pomocí tabulky s rozptýlenými položkami)
// autoři:         Boris Burkalo, xburka00, 2BIT
//                 Jiří Herrgott, xherrg00, 2BIT
//                 Jan Klusáček, xklusa14, 2BIT
*/

#include "parser.h"
#include "generator.h"
#define BRACKET_RETURN 800
#define NO_ERROR_STAT 900


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
extern hSymtab *table;
const char *operNamesP[] = {"+", "-", "*", "/", "//", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "new line", "keyword", "variable", "string", "documentary string", "int", "float", "indent", "dedent", "None", "pass", "unspecified", "func", "undefined"};

int if_else_flag = 0;
int var_flag = 0;
int no_ret_flag;
hSym_fct_stack fct_predef_stack;
int indent_counter = 0;
int dedent_counter = 0;


int print_fct_call(Token *token, hSymtab *act_table, int in_function){
  Print_Stack p_stack;
  p_stack.top = -1;
  Type prev = TypeComma;

  if (GET_TOKEN_CHECK_EOF(token) && TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)) {

    //generete if there are no params
    return NO_ERROR;
  }

  p_stack.term = malloc(sizeof(Token) * 10); /// proc x deset


  while (token->type != TypeRightBracket) {
    switch(token->type){
      case TypeVariable:
        if (symtab_it_position((char*)token->data, act_table)) {
          if (symtab_it_position((char*)token->data, act_table)->item_type == IT_VAR){
            p_stack.top++;
            p_stack.term[p_stack.top] = *token;
            prev = token->type;
          }
          else{
            return ERROR_SEMANTIC;
          }
        }
        else {
          return ERROR_SEMANTIC;
        }
        break;
      case TypeString:
      case TypeInt:
      case TypeFloat:
        p_stack.top++;
        p_stack.term[p_stack.top] = *token;
        prev = token->type;
        break;

      case TypeDocString:
        p_stack.top++;
        p_stack.term[p_stack.top] = *token;
        p_stack.term[p_stack.top].type = TypeString;
        prev = token->type;
        break;
      case TypeComma:
        if (prev == TypeComma) {
          return ERROR_SYNTAX;
        }
        prev = TypeComma;
        break;
      default:
        return ERROR_SYNTAX;
    }

    if (GET_TOKEN_CHECK_EOF(token) && TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)) {

        if (prev != TypeComma){
          //generete if there are no params
          return NO_ERROR;
        }
        else {
          return ERROR_SYNTAX;
        }
    }

  }

  // POSILANI DO GENERATORU
  generate_fnc_pre_param();
  int push_num = p_stack.top + 1;
  while(p_stack.top != -1){
    switch (p_stack.term[p_stack.top].type) {
      case TypeVariable:
        generate_push_var(p_stack.term[p_stack.top].data, !in_function);
        p_stack.top--;
        break;

      case TypeInt:
      case TypeString:
      case TypeFloat:
        generate_push_data(p_stack.term[p_stack.top].type, p_stack.term[p_stack.top].data);
        p_stack.top--;
        break;
      default:
        return ERROR_SYNTAX;

    }
  }
  generate_fnc_param_set_data(TypeInt , &push_num, 0);
  generate_fnc_call("print");
  return NO_ERROR;


}

// Basic without expressions yet // add code generating
int fction_call(Token *token, hSymtab *act_table, int in_function){
  Type prev;

  if (!strcmp((char*)token->data, "print")) {
    err = print_fct_call(token, act_table, in_function);
    return err;

  }


  int param_cntr = 0;
  hSymtab_it *tmp_item_func = symtab_it_position((char*)token->data, act_table);
  if (!tmp_item_func && in_function) {

    fct_predef_stack.top = sym_stack_push(fct_predef_stack.top, (char*)token->data);
    if (!fct_predef_stack.top) {
      return ERROR_INTERNAL;
    }



    if (!GET_TOKEN_CHECK_EOF(token)) {
      if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)){
        fct_predef_stack.top->param_num = param_cntr;
        return NO_ERROR;
      }
    }
    prev = TypeComma;


    while (token->type != TypeRightBracket) {


      if (token->type == TypeVariable) {
        if (!symtab_it_position((char*)token->data, act_table)) {
          return ERROR_SEMANTIC;
        }
        param_cntr++;
      }
      else if (token->type == TypeComma && prev == TypeComma){
        return ERROR_SYNTAX;
      }
      else if (token->type == TypeString || token->type == TypeInt || token->type == TypeFloat){
        param_cntr++;
      }

      prev = token->type;

      if (GET_TOKEN_CHECK_EOF(token)){
        return ERROR_SYNTAX;
      }
    }


    fct_predef_stack.top->param_num = param_cntr;


    return NO_ERROR;


  }
  else if (tmp_item_func) {
    if (tmp_item_func->item_type == IT_FUNC) {

      generate_fnc_pre_param();

      hSymtab_Func_Param *tmp_params = ((hSymtab_Func*)tmp_item_func->data)->params;
      if (((hSymtab_Func*)tmp_item_func->data)->paramNum == 0) {
        // SPATNE NEMUZE BYT EOF TODO
        if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)) {
          //generete if there are no params
          generate_fnc_call(tmp_item_func->hKey);
          return NO_ERROR;
        }
        else {
          return ERROR_SEMANTIC_FUNCTION_CALL;
        }
      }
      else {
        if (GET_TOKEN_CHECK_EOF(token)) {
          return ERROR_SYNTAX;
        }
        bool var_flag = false;
        while (1){
          var_flag = 0;
          if (token->type == TypeComma) {
            if (prev == TypeComma) {
              return ERROR_SYNTAX;
            }
            prev = TypeComma;

            if (GET_TOKEN_CHECK_EOF(token)) {
              return ERROR_SYNTAX;
            }
          }
          else if (token->type == TypeRightBracket){
            if (((hSymtab_Func*)tmp_item_func->data)->paramNum == param_cntr) {
              return NO_ERROR;
            }

            return ERROR_SEMANTIC_FUNCTION_CALL;
          }
          else {
            if (token->type == TypeVariable) {
              if(symtab_it_position((char*)token->data, act_table)){
                if (symtab_it_position((char*)token->data, act_table)->item_type == IT_VAR) {
                  token->type = ((hSymtab_Var*)symtab_it_position((char*)token->data, act_table)->data)->type;
                  prev = TypeVariable;
                  var_flag = true;
                }
                else {
                  return ERROR_SYNTAX;
                }
              }
              else {
                return ERROR_SEMANTIC;
              }
            }

            if (tmp_params->param_type == TypeUnspecified) {

              if (token->type == TypeString || token->type == TypeFloat || token->type == TypeInt || token->type == TypeUnspecified) {
                  if(!var_flag)
                    prev = token->type;

                  if(var_flag)
                      generate_fnc_param_set_var(token->data, ((hSymtab_Var*)symtab_it_position((char*)token->data, act_table)->data)->global, param_cntr);
                  else
                      generate_fnc_param_set_data(token->type, token->data, param_cntr);

                if (GET_TOKEN_CHECK_EOF(token)) {
                  return ERROR_SYNTAX;
                }
                if (!tmp_params->next) {
                  if (token->type != TypeRightBracket) {
                    return ERROR_SEMANTIC_FUNCTION_CALL;
                  }
                  else {

                    // generate code
                    generate_fnc_call(tmp_item_func->hKey);
                    return NO_ERROR;
                  }
                }
                else{
                  if (token->type == TypeRightBracket) {
                    return ERROR_SEMANTIC_FUNCTION_CALL;
                  }
                }


                // generator
                param_cntr++;
                tmp_params = tmp_params->next;
              }
            }
            else{
              if (token->type != tmp_params->param_type) {

                //////////////TODO////////////////
                return ERROR_SEMANTIC_RUNTIME;
                //////////////////////////////////

              }
              else {
                  if(!var_flag)
                    prev = token->type;

                  if(var_flag)
                      generate_fnc_param_set_var(token->data, ((hSymtab_Var*)symtab_it_position((char*)token->data, act_table)->data)->global, param_cntr);
                  else
                      generate_fnc_param_set_data(token->type, token->data, param_cntr);

                if (GET_TOKEN_CHECK_EOF(token)) {
                  return ERROR_SYNTAX;
                }

                if (!tmp_params->next) {
                  if (token->type != TypeRightBracket) {
                    return ERROR_SEMANTIC_FUNCTION_CALL;
                  }
                  else {

                    //last parameter
                    generate_fnc_call(tmp_item_func->hKey);
                    return NO_ERROR;
                  }
                }
                else{
                  if (token->type == TypeRightBracket) {
                    return ERROR_SEMANTIC_FUNCTION_CALL;
                  }
                }

                //generator
                param_cntr++;
                tmp_params = tmp_params->next;
              }
            }
          }
        }
      }
    }
  }
  // Function was not defined yet and is called in global scope
  else {
    return ERROR_SEMANTIC;
  }
  return ERROR_INTERNAL;
}


int statement_body(Token *token, hSymtab *act_table, int in_function, char* fction_name, size_t pos){

  if (GET_TOKEN_CHECK_EOF(token)) {
    return NO_ERROR;
  }

  while (token->type != TypeDedend) {
    if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)) {
      if_else_flag = 1;
      return NO_ERROR;
    }

    err = command(token, act_table, in_function, fction_name, 1, pos);

    if (err != NO_ERROR) {
      return err;
    }


    if (token->type == TypeDedend){
      if (GET_TOKEN_CHECK_EOF(token)) {
        return NO_ERROR;
      }

      if (!strcmp((char*)token->data, "else")) {
        indent_counter--;
        return NO_ERROR;
      }

      continue;
    }


    if (GET_TOKEN_CHECK_EOF(token)) {
      return NO_ERROR;
    }

  }

  indent_counter--;

  return NO_ERROR;
}


int statement(Token *token, hSymtab *act_table, int in_function, char* fction_name, size_t pos){
  bool else_ = false;
  bool if_ = false;
  bool while_ = false;


  if (!strcmp((char*)token->data, "if")){
    if_ = true;
  }
  else if(!strcmp((char*)token->data, "while")) {
    while_ = true;
  }
  else if (!strcmp((char*)token->data, "else")) {
    return ERROR_SYNTAX;
  }

  if (GET_TOKEN_CHECK_EOF(token)) {
    exit(1);
  }



  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeFloat) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeString) ||
      TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeLeftBracket)) {

    char uql[MAX_DIGITS_DOUBLE];
    size_t pos_tmp = 0;
      if (if_)
      {
          char uql[MAX_DIGITS_DOUBLE];
          generate_unique_label(uql, LABEL_IF);
      }
      else if(while_)
      {
        char uql[MAX_DIGITS_DOUBLE];
        generate_unique_label(uql, LABEL_WHILE);
        generate_while_begin(uql, &pos_tmp);
      }

    err = expression(NULL, token, NULL, act_table);

    if (err != NO_ERROR) {
      return err;
    }


    if (!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)) {
      return ERROR_SYNTAX;
    }


    if (GET_TOKEN_CHECK_EOF(token)) {
      return ERROR_SYNTAX;
    }


    if (!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
      return ERROR_SYNTAX;
    }

    if (GET_TOKEN_CHECK_EOF(token) || !TOKEN_TYPE_NEEDED_CHECK(token->type, TypeIndent)) {
      return ERROR_SYNTAX;
    }

    indent_counter++;

    generate_pop_exp();

    if (if_)
    {
        generate_if_begin(uql, &pos_tmp);
    }
    else if(while_)
    {
        generate_while_loop(uql);
    }

    if(pos == 0)
    {
        pos = pos_tmp;
    }

    err = statement_body(token, act_table, in_function, fction_name, pos);

    if(err)
        return err;

    if (strcmp((char*)token->data, "else") || token->type == TypeDedend){
      if(GET_TOKEN_CHECK_EOF(token)){
        return NO_ERROR;
      }
    }

    if (!strcmp((char*)token->data, "else")) {
      if (if_) {
        else_ = true;
        generate_else(uql);

        if(GET_TOKEN_CHECK_EOF(token)){
          return NO_ERROR;
        }


        if (token->type != TypeColon) {
          return ERROR_SYNTAX;
        }

        if (GET_TOKEN_CHECK_EOF(token)) {
          return ERROR_SYNTAX;
        }

        if (!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
          return ERROR_SYNTAX;
        }


        if (GET_TOKEN_CHECK_EOF(token) || !TOKEN_TYPE_NEEDED_CHECK(token->type, TypeIndent)) {
          return ERROR_SYNTAX;
        }
        indent_counter++;

        err = statement_body(token, act_table, in_function, fction_name, pos);

        if (err != NO_ERROR) {
          return err;
        }

      }
      else {
        return ERROR_SYNTAX;
      }
    }
    else {

      if (if_)
      {
          generate_if_end(uql, else_);
      }
      else if(while_)
      {
          generate_while_end(uql);
      }

      if (token->type == TypeDedend) {
        indent_counter--;
        return NO_ERROR;
      }


      err = command(token, act_table, in_function, fction_name, in_function, 0);
      return err;
    }

    if (if_)
    {
        generate_if_end(uql, else_);
    }
    else if(while_)
    {
        generate_while_end(uql);
    }
    //if_else_flag = if_;

    return err;


}
else {
  return ERROR_SYNTAX;
}

  return ERROR_INTERNAL;

}




int assignment(Token *var, Token *value, hSymtab *act_table, int in_function, size_t pos){


  if (GET_TOKEN_CHECK_EOF(value) || TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {
    exit(1);
  }

  bool global = true;
    if (!symtab_it_position((char*)var->data, act_table))
    {
      symtab_add_it(act_table, var);
      ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->global = true;

      if(pos > 0)
      {
        generate_var_declaration_on_pos((char*)var->data, !in_function, pos);
      }
      else
      {
        generate_var_declaration((char*)var->data, !in_function);
      }
      if(in_function)
          global = false;
    }
    else
    {
      if (symtab_it_position((char*)var->data, act_table)->item_type == IT_VAR)
        global = ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->global;
      else
        return ERROR_SEMANTIC;
    }

    if (value->type == TypeVariable) {

      if (symtab_it_position((char*)value->data, act_table)) {
        if (symtab_it_position((char*)value->data, act_table)->item_type == IT_FUNC) {
          Token bracket;

          if (GET_TOKEN_CHECK_EOF(&bracket) || !TOKEN_TYPE_NEEDED_CHECK(bracket.type, TypeLeftBracket)){
            return ERROR_SYNTAX;
          }

          if (((hSymtab_Func*)((*act_table)[symtab_hash_function((char*)value->data)]->data))->return_type != TypeUndefined) {
            ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->type = ((hSymtab_Func*)((*act_table)[symtab_hash_function((char*)value->data)]->data))->return_type;
          }

            err = fction_call(value, act_table, in_function);
            if (err != NO_ERROR) {
              return err;
            }


            generate_fnc_return_get(var->data, global);

            if (GET_TOKEN_CHECK_EOF(value) || !TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)){
              return ERROR_SYNTAX;
            }


            ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->defined = true;
            return err;
        }
      }
    }

    Token tmp_func = *value;

    err = expression(NULL ,value, (*act_table)[symtab_hash_function((char*)var->data)], act_table);


    if (err == BRACKET_RETURN) {
      err = fction_call(&tmp_func, act_table, in_function);

      if (err != NO_ERROR) {
        return err;
      }

      generate_fnc_return_get(var->data, global);

      if (GET_TOKEN_CHECK_EOF(value)){
        return NO_ERROR;
      }

      if(!TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)){
        return ERROR_SYNTAX;
      }


      ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->defined = true;
      return err;
    }

    if (err != NO_ERROR) {
      return err;
    }

    if (!TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {
      return ERROR_SYNTAX;
    }
    ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->defined = true;

    if(symtab_it_position((char*)var->data, act_table)->item_type == IT_VAR)
        generate_pop_var((char*)var->data, global);

    return err;
}


int command(Token *token, hSymtab *act_table, int in_function, char* fction_name, int statement_switch, size_t pos){



  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {
    if_else_flag = 0;
    Token token_n;

    if (GET_TOKEN_CHECK_EOF(&token_n)) {
      return NO_ERROR;
    }


    if (TOKEN_TYPE_NEEDED_CHECK(token_n.type, TypeAssignment)) {


      err = assignment(token, &token_n, act_table, in_function, pos);
      if (in_function == 1 && !symtab_it_position((char*)token->data, table)) {
        ((hSymtab_Var*)symtab_it_position((char*)token->data, act_table)->data)->global = false;
      }
      return err;


    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token_n.type, TypeLeftBracket)){

      // funguje pouze s globalni tabulkou // TODO

      err = fction_call(token, act_table, in_function);
      return err;
    }
    else {
      err = expression(token, &token_n, NULL, act_table);
      return err;
    }
  }
  else if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeString) ||
          TOKEN_TYPE_NEEDED_CHECK(token->type, TypeFloat)) {
    if_else_flag = 0;
    err = expression(NULL, token, NULL, act_table);
    return err;
  }
  else if ((char*)token->data && strcmp((char*)token->data, "def") == 0 && statement_switch == 0) {

    if(in_function == 1){ //def cannot be in fuction
      return ERROR_SYNTAX;
    }
    free(token->data);//po tomhle ifu useless

    int fction_start_return = fction_start(token, act_table);
    if(fction_start_return != NO_ERROR){
      return fction_start_return;
    }

    if (GET_TOKEN_CHECK_EOF(token)){
      return ERROR_SYNTAX;
    }

    err = body(token, act_table);
    return err;
    //}

  } // Pridat None a Pass
  else if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeKeyword)) {

    if (!strcmp((char*)token->data, "return") && in_function) {

      if (GET_TOKEN_CHECK_EOF(token)) {
        return NO_ERROR;
      }

      err = expression(NULL, token, NULL, act_table);
      generate_pop_return();
      generate_fnc_return(fction_name);

      return err;
    }

    err = statement(token, act_table, in_function, fction_name, pos);
    return err;
  }
  else if (token->type == TypeDocString){
    if (GET_TOKEN_CHECK_EOF(token)){
      return NO_ERROR;
    }
    return NO_ERROR;
  }
  else if (token->type == TypeKeywordPass) {
    if (GET_TOKEN_CHECK_EOF(token)){
      return NO_ERROR;
    }
    return NO_ERROR;
  }
  else if (token->type == TypeNewLine)
    return NO_ERROR;
  else {
    return ERROR_SYNTAX;
  }

  return ERROR_INTERNAL;
}

int body(Token *token, hSymtab *act_table){
  while (1) {
      if (err == NO_ERROR) {
        err = command(token, act_table, 0, NULL, 0, 0);


        if (err == ERROR_SYNTAX || err == ERROR_SEMANTIC) {
          return err;
        }
      }
      else{
        return err;
      }


      if (GET_TOKEN_CHECK_EOF(token)) {
        return err;
      }

      if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
        if (GET_TOKEN_CHECK_EOF(token)) {
          return err;
        }
      }

  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//functions

int fction_params(Token *token, hSymtab_it *symtab_it){
  //need new token, now token is left bracket
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
   return ERROR_SYNTAX;
  }

  int check_comma = 0; //params should not end with comma
  bool malloc_check = false; //we want malloc only fist time
  hSymtab_Func_Param *params = NULL;
  unsigned param_counter = 0;

  while(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)){
    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
      check_comma = 0;
      if (malloc_check == false){

        if( !(((hSymtab_Func *)(symtab_it->data))->params = malloc(sizeof(hSymtab_Func_Param))) ){
          return ERROR_INTERNAL; //malloc error
        }

        params = ((hSymtab_Func *)(symtab_it->data))->params;
        ((hSymtab_Func *)(symtab_it->data))->params->param_type = TypeUnspecified;
        params->next = NULL;
        params->paramName = (char*)token->data;
        generate_fnc_param_get(params->paramName, param_counter);


        malloc_check = true;
      }
      //after malloc
      else {
        //for name check
        hSymtab_Func_Param *names = NULL;
        names = ((hSymtab_Func *)symtab_it->data)->params;

        while(names != NULL){
          if(strcmp(names->paramName, (char*)token->data) == 0){
            //error two parameters with same name
            return ERROR_SEMANTIC;
          }
          if (!names->next) {
            break;
          }
            names = names->next;
        }


        if( !(params->next = malloc(sizeof(hSymtab_Func_Param))) ){
          return ERROR_INTERNAL;
        }

        params->next->param_type = TypeUnspecified;
        params->next->next = NULL;


        params->next->paramName = (char*)token->data;
        generate_fnc_param_get(params->next->paramName, param_counter);
        params = params->next;
        params->next = NULL;
      }
      param_counter++;
    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeComma)){
      check_comma = 1;
    }
    else{
      //param není variable nebo comma
      return ERROR_SYNTAX;
    }
    //get next token
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
      return ERROR_SYNTAX;
    }
  }
  if (check_comma == 1){
    //add free
    //parametry končí čárkou
    return ERROR_SYNTAX;
  }
  else{
    //parametry vypadají v pořádku
    ((hSymtab_Func *)(symtab_it->data))->paramNum = param_counter;
    return NO_ERROR;
  }
}

int fction_body(Token *token, hSymtab_it *symtab_it){
  indent_counter = 0, dedent_counter = 1; //last dedent is checked after this function
  no_ret_flag = 0;

  hSymtab local_table;
  symtab_init(&local_table);
  symtab_copy(table, &local_table);



  //to copy params to local_table
  hSymtab_Func_Param *params = NULL;
  params = ((hSymtab_Func *)(symtab_it->data))->params;
  Token param;

  while(params != NULL){
    param.data = params->paramName;
    param.type = TypeVariable;
    symtab_add_it(&local_table, &param);
    ((hSymtab_Var*)symtab_it_position((char*)param.data, &local_table)->data)->defined = true;
    ((hSymtab_Var*)symtab_it_position((char*)param.data, &local_table)->data)->global = false;
    params = params->next;
  }

  //if there is indent, it is ok
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeIndent)){
    indent_counter++;
    if (GET_TOKEN_CHECK_EOF(token)) {
      return ERROR_SYNTAX;
    }

      while (strcmp((char*)token->data, "return") != 0) {
          if (err == NO_ERROR) {
            if((err = command(token, &local_table, 1, symtab_it->hKey, 0, 0)) == ERROR_SYNTAX){
              return ERROR_SYNTAX;
            }
            if (err != NO_ERROR) {
              return err;
            }
          }
          else{
            return err;
          }

          if (indent_counter == 0) {
            no_ret_flag = 1;

            //add return type
            free_symtab(&local_table, 1);
            return 1000;
          }
          //EOF in body of function
          if (GET_TOKEN_CHECK_EOF(token)) {
            return ERROR_SYNTAX;
          }
          if (token->type == TypeDedend) {
            no_ret_flag = 1;

            //add return type
            free_symtab(&local_table, 1);
            return 1000;
          }

      }
      free(token->data);//smaze "return"
      if(indent_counter == dedent_counter){

        //code ends with definition
        if(GET_TOKEN_CHECK_EOF(token)) return 0;


        if(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
            //add return type
            err = expression(NULL, token, symtab_it, &local_table);
            generate_pop_return();
            generate_fnc_return(symtab_it->hKey);
            free(token->data);//po zjisteni varu nazev smaze
            free_symtab(&local_table, 1);
            return 1000;
        }
        //no return value
        else{
          free_symtab(&local_table, 1);
          return 1000;
        }
      }
      else{
        //INDENT != DEDENT
        free_symtab(&local_table, 1);
        return ERROR_SYNTAX;
      }
  }
  else{
    //no indent
    free_symtab(&local_table, 1);
    return ERROR_SYNTAX;
  }
}


int fction_start(Token *token, hSymtab *act_table){

  if (GET_TOKEN_CHECK_EOF(token)) {
    return ERROR_SYNTAX;
  }

  //beginning of function
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
    Token fction_name = *token;
    generate_fnc_begin(token->data);

    //check if next token is left bracket and not EOF or newline
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
      return ERROR_SYNTAX;
    }

    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeLeftBracket)){
      //check if function with same name already exists
      if(symtab_it_position((char*)token->data, act_table) == NULL){

        fction_name.type = TypeFunc;
        symtab_add_it(act_table, &fction_name); //add name of fction to act_table

        //for easier check of retrun value of params
        int fction_return = fction_params(token, symtab_it_position((char *)fction_name.data, act_table));

        if(fction_return == ERROR_SYNTAX){
          return ERROR_SYNTAX;
        }
        else if(fction_return == ERROR_INTERNAL){
          return ERROR_INTERNAL;
        }
        else if(fction_return == ERROR_SEMANTIC){
          return ERROR_SEMANTIC;
        }
        //NO ERROR

        //get new token
        if (GET_TOKEN_CHECK_EOF(token)) {
          return ERROR_SYNTAX;
        }
        //is there ":"" and EOF after ")"?
        if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)){
          if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
            //konec hlavičky
            //generate???
            if (GET_TOKEN_CHECK_EOF(token)) {
              return ERROR_SYNTAX;
            }
            //for easier check of return values form body of function
            int fction_body_return = fction_body(token, symtab_it_position((char *)fction_name.data, act_table));
            //after return check dedent
            //function has return value


            //sterv generate_fnc_end(char* label)

            if (fction_body_return == 0){
              //newline
              if (GET_TOKEN_CHECK_EOF(token)) {
                return ERROR_SYNTAX;
              }

              //check dedent or eof
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                generate_fnc_end(fction_name.data);
                free(fction_name.data);//dal uz nebude potreba
                return NO_ERROR;
              }
              //po returnu není dedent
              else{
                return ERROR_SYNTAX;
              }
            }
            //function does not have return value
            else if(fction_body_return == 1000){

              // bez returnu
              if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend) && no_ret_flag){
                generate_fnc_end(fction_name.data);
                free(fction_name.data);//dal uz nebude potreba
                return NO_ERROR;
              }

              //check dedent or eof
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                generate_fnc_end(fction_name.data);
                free(fction_name.data);//dal uz nebude potreba
                return NO_ERROR;
              }
              //po returnu není dedent
              else{
                return ERROR_SYNTAX;
              }
            }
            else {
              return err;
            }
          }
          //po ":" není EOF
          return ERROR_SYNTAX;
        }
        //po pravé závorce není ":"
        else{
          return ERROR_SYNTAX;
        }
      }
      else{
        return ERROR_SEMANTIC; //function already exists
      }
    }
    //po názvu funkce nenásleduje "("
    return ERROR_SYNTAX;
  }
  //po def není TypeVariable
  else{
    return ERROR_SYNTAX;
  }
}
//end of functions
///////////////////////////////////////////////////////////////////////////////////////////////////////

// FOR NOW EQUAL TO <prog>
// <prog> -> <body>
// <prog> -> <fction_start> INDENT <body> DEDENT <body>
int prog() {
  Token token;
  scanner_init();
  generator_init();

  fct_predef_stack.top = malloc(sizeof(hSym_fct_node));
  if (!fct_predef_stack.top) {
    return ERROR_INTERNAL;
  }
  fct_predef_stack.top->fct_name = NULL;


  table = malloc(sizeof(hSymtab));
  if (!table) {
    return ERROR_INTERNAL;
  }
  symtab_init(table);
  symtab_add_predef_func(table);

  //proc to bylo driv tak slozite?
  generate_main_begin();

  if(GET_TOKEN_CHECK_EOF(&token)){
      generate_main_end();
      return NO_ERROR;
  }



  body(&token, table);
  if (err == NO_ERROR){
    err = sym_stack_pop_all(fct_predef_stack.top, table);
  }


  generate_main_end();

  return err;
}
