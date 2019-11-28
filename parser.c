#include "parser.h"
#include "generator.h"

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
const char *operNames[];


// Basic without expressions yet // add code generating
int fction_call(Token *token, hSymtab *act_table){
  Type prev;

  unsigned param_cntr = 0;
  hSymtab_it *tmp_item_func = symtab_it_position((char*)token->data, act_table);
  if (tmp_item_func) {
    if (tmp_item_func->item_type == IT_FUNC) {

      generate_fnc_pre_param();

      hSymtab_Func_Param *tmp_params = ((hSymtab_Func*)tmp_item_func->data)->params;
      //int act_parameters_cnt = symtab_num_of_fction_params(tmp_params);
      if (!tmp_params) {
        if (GET_TOKEN_CHECK_EOF(token) && TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)) {

          //generete if there are no params
          generate_fnc_call(tmp_item_func->hKey);
          return NO_ERROR;
        }
      }
      else {
        if (GET_TOKEN_CHECK_EOF(token)) {
          return ERROR_SYNTAX;
        }
        bool var_flag = false;
        while (1){
          //printf("%s\n", operNames[token->type]);
          if (token->type == TypeComma) {
            if (prev == TypeComma) {
              return ERROR_SYNTAX;
            }
            prev = TypeComma;

            if (GET_TOKEN_CHECK_EOF(token)) {
              return ERROR_SYNTAX;
            }
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
                    return ERROR_SEMANTIC;
                  }
                  else {

                    // generate code
                    generate_fnc_call(tmp_item_func->hKey);
                    return NO_ERROR;
                  }
                }
                else{
                  if (token->type == TypeRightBracket) {
                    return ERROR_SEMANTIC;
                  }
                }


                // generator
                param_cntr++;
                tmp_params = tmp_params->next;
              }
            }
            else{
              if (token->type != tmp_params->param_type) {
                return ERROR_SEMANTIC;
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
                    return ERROR_SEMANTIC;
                  }
                  else {

                    //last parameter
                    generate_fnc_call(tmp_item_func->hKey);
                    return NO_ERROR;
                  }
                }
                else{
                  if (token->type == TypeRightBracket) {
                    return ERROR_SEMANTIC;
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
}


int statement_body(Token *token, hSymtab *act_table){
  while (token->type != TypeDedend) {

    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF successfully\n"); return NO_ERROR;}

    if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)) {
      return NO_ERROR;
    }

    err = command(token, act_table, 0, 1);
    if (err != NO_ERROR) {
      return err;
    }
  }
}


int statement(Token *token, hSymtab *act_table){

  int indent_cnt = 0;

  if (!strcmp((char*)token->data, "if")){
    printf("\t \tSENT TO GENERATOR: %s\n", (char*)token->data);
  }
  else if(!strcmp((char*)token->data, "while")) {
    printf("\t \tSENT TO GENERATOR: %s\n", (char*)token->data);
  }

  if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  printf("%d\n", TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt));


  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeFloat) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeString) ||
      TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {
    err = expression(NULL, token, NULL, act_table);

    if (!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)) {
      DEBUG_PRINT("Syntax error: Colon is missing\n");
      return ERROR_SYNTAX;
    }

    if (GET_TOKEN_CHECK_EOF(token)) {
      DEBUG_PRINT("Reached EOF where it should not be. \n");
      return ERROR_SYNTAX;
    }

    if (!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
      DEBUG_PRINT("Syntax error: Newline is missing\n");
      return ERROR_SYNTAX;
    }


    if (GET_TOKEN_CHECK_EOF(token) || !TOKEN_TYPE_NEEDED_CHECK(token->type, TypeIndent)) {
      DEBUG_PRINT("Reached EOF where it should not be or indent missing. \n");
      return ERROR_SYNTAX;
    }

    err = statement_body(token, act_table);
    return err;
    printf("we good\n");
    exit(0);


  }
  else if (!strcmp((char*)token->data, "None")){

  }

}




int assignment(Token *var, Token *value, hSymtab *act_table, int in_function){

  // GENERATE INSTRUCTION //
    printf("\t \tSENT TO GENERATOR: %s\n", (char*)var->data);
    printf("\t \tSENT TO GENERATOR: %s\n", operNames[value->type]);
  ///

  if (GET_TOKEN_CHECK_EOF(value) || TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  hSymtab_it *tmp_item_var;
  hSymtab_it *tmp_item_value;

  bool global = true;
    if (!symtab_it_position((char*)var->data, act_table))
    {
      symtab_add_it(act_table, var);
      ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->global = true;
      generate_var_declaration((char*)var->data, !in_function);
      if(in_function)
          global = false;
    }
    else
    {
        global = ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->global;
    }

    if (value->type == TypeVariable) {
      /* ERROR SEG FAULT */

      if (symtab_it_position((char*)value->data, act_table)) {
        if (symtab_it_position((char*)value->data, act_table)->item_type == IT_FUNC) {
          Token bracket;

          if (GET_TOKEN_CHECK_EOF(&bracket) || !TOKEN_TYPE_NEEDED_CHECK(bracket.type, TypeLeftBracket)){
            return ERROR_SYNTAX;
          }

            err = fction_call(value, act_table);

            generate_fnc_return_get(var->data, global);

            if (GET_TOKEN_CHECK_EOF(value) || !TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)){
              return ERROR_SYNTAX;
            }


            return err;
        }
      }
    }


    err = expression(NULL ,value, (*act_table)[symtab_hash_function((char*)var->data)], act_table);
    if (err == ERROR_SYNTAX || err == ERROR_SEMANTIC) {
      return err;
    }

    if (!TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {
      DEBUG_PRINT("Colon not newline\n");
      return ERROR_SYNTAX;
    }
    ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->defined = true;

    if(symtab_it_position((char*)var->data, act_table)->item_type == IT_VAR)
        generate_pop_var((char*)var->data, global);
    //else
        //generate_pop_param(unsigned index);

    return err;
}


int command(Token *token, hSymtab *act_table, int in_function, int statement_switch){


  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {
    Token token_n;

    if (GET_TOKEN_CHECK_EOF(&token_n)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}



    if (TOKEN_TYPE_NEEDED_CHECK(token_n.type, TypeAssignment)) {

      err = assignment(token, &token_n, act_table, in_function);
      if (in_function == 1 && !symtab_it_position((char*)token->data, table)) {
        ((hSymtab_Var*)symtab_it_position((char*)token->data, act_table)->data)->global = false;
      }
      return err;


    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token_n.type, TypeLeftBracket)){

      // funguje pouze s globalni tabulkou // TODO
      err = fction_call(token, act_table);
      return err;
    }
    else {
      err = expression(token, &token_n, NULL, act_table);
      return err;
    }
  }
  else if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeInt) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeString) ||
          TOKEN_TYPE_NEEDED_CHECK(token->type, TypeFloat)) {
    err = expression(NULL, token, NULL, act_table);
    return err;
  }
  else if (strcmp((char*)token->data, "def") == 0 && statement_switch == 0) {

    if(in_function == 1){ //def cannot be in fuction
      //printf("kokot\n");
      return ERROR_SYNTAX;
    }
    free(token->data);//po tomhle ifu useless

    int fction_start_return = fction_start(token, act_table);
    if(fction_start_return == NO_ERROR){
      //printf("jsem zpátky v body/command\n\n\n");
    }
    else if(fction_start_return == ERROR_SYNTAX){
      return ERROR_SYNTAX;
    }
    else if(fction_start_return == ERROR_SEMANTIC){
      return ERROR_SEMANTIC;
    }
    else if(fction_start_return == ERROR_INTERNAL){
      return ERROR_INTERNAL;
    }
    else{ //nemělo by se sem dostat
      //printf("chyba, neměl bych se sem dostat\n");
      return ERROR_INTERNAL;
    }
    //tady upravit aby prošlo, když vstupní kód končí definicí
    /*if (in_function == 1){
      return;
    }
    else{*/
    if (GET_TOKEN_CHECK_EOF(token)){DEBUG_PRINT("Reached EOF after function definition\n"); return 0;}//exit(1);}
    body(token, act_table);
    //}

  } // Pridat None a Pass
  else if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeKeyword)) {
      err = statement(token, act_table);
      return err;
  }
  else {
    return ERROR_SYNTAX;
  }
}

int body(Token *token, hSymtab *act_table){
  while (1) {
      if (err == NO_ERROR) {
        err = command(token, act_table, 0, 0);

        if (err == ERROR_SYNTAX || err == ERROR_SEMANTIC) {
          DEBUG_PRINT("Error occured, parsing ended.\n");
          return err;
        }
      }
      else{
        DEBUG_PRINT("Found EOF, parsing terminated.\n");
        return err;
      }

      if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return err;}

      if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {
        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return err;}
      }

  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//functions

int fction_params(Token *token, hSymtab_it *symtab_it){
  //need new token, now token is left bracket
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); return ERROR_SYNTAX;}
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
/*
       if( !(((hSymtab_Func *)(symtab_it->data))->params->paramName = malloc(sizeof(char)*strlen((char*)token->data))) ){
          return 99; //malloc error
        }
        strcpy(params->paramName, (char*)token->data);*/
        params->paramName = (char*)token->data;
        generate_fnc_param_get(params->paramName, param_counter);


        //sterv params->paramName = (char*)token->data;
        //no need to check name, it is first param

        malloc_check = true;
      }
      //after malloc
      else {
        //for name check
        hSymtab_Func_Param *names = NULL;
        names = ((hSymtab_Func *)symtab_it->data)->params;
        hSymtab_Func_Param *tmp = NULL;

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

        //printf("%s\n", (char*)token->data);

        if( !(params->next = malloc(sizeof(hSymtab_Func_Param))) ){
          return ERROR_INTERNAL;
        }

        params->next->param_type = TypeUnspecified;
        params->next->next = NULL;

/*
        if( !(params->next->paramName = malloc(sizeof(char)*strlen((char*)token->data))) ){
          return 99;
        }



        strcpy(params->next->paramName, (char*)token->data);
*/
        /* sterv bool generate_fnc_param_get(char* label, unsigned index); -> param_counter
        label je nazev parametru, a index je cislo parametru
        nazev by mel byt (char*)token->data*/

        //sterv params->paramName = (char*)token->data;
        params->next->paramName = (char*)token->data;
        generate_fnc_param_get(params->next->paramName, param_counter);
        params = params->next;
        params->next = NULL;
      }
      param_counter++;
    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeComma)){
      //printf("%d\n", token->type);
      check_comma = 1;
    }
    else{
      //param není variable nebo comma
      return ERROR_SYNTAX;
    }
    //printf("%d\n", token->type);
    //get next token
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); return ERROR_SYNTAX;}
    //printf("%d\n", token->type);
  }
  if (check_comma == 1){
    //add free
    //parametry končí čárkou
    return ERROR_SYNTAX;
  }
  else{
    //parametry vypadají v pořádku
    return NO_ERROR;
  }
}

int fction_body(Token *token, hSymtab_it *symtab_it){
  int indent_counter = 0, dedent_counter = 1; //last dedent is checked after this function

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
    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); return ERROR_SYNTAX;}

      while (strcmp((char*)token->data, "return") != 0) {
          if (err == NO_ERROR) {
            if(command(token, &local_table, 1, 0) == ERROR_SYNTAX){
              return ERROR_SYNTAX;
            }
            if (err == ERROR_SYNTAX || err == ERROR_SEMANTIC) {
              DEBUG_PRINT("Error occured, parsing ended.\n");
              return err;
            }
          }
          else{
            DEBUG_PRINT("Found EOF, parsing terminated.  %s\n", (char *)token->data);
            return err;
          }
          //EOF in body of function
          if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return ERROR_SYNTAX;}

      }
      free(token->data);//smaze "return"
      if(indent_counter == dedent_counter){
        //doplnit return type

        //code ends with definition
        if(GET_TOKEN_CHECK_EOF(token)) return 0;
        //return value is variable or something

        if(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
          //variable
         // if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
            /*if(symtab_it_position((char*)token->data, &local_table) == NULL){
              return ERROR_SEMANTIC; //return variable is not in local_table of function
            }
            else{

              printf("\n\nLOCAL----------------------->\n");
              print_sym_tab(&local_table);
              printf("<-------------------------END\n\n\n");
              //add return type
              err = expression(NULL, token, symtab_it, &local_table);
              free(token->data);//po zjisteni varu nazev smaze
              free_symtab(&local_table, 1);
              return 1000;
              }
          }*/
          //something else, need fix for only possible return types
          //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          //else{
            printf("\n\nLOCAL----------------------->\n");
            print_sym_tab(&local_table);
            printf("<-------------------------END\n\n\n");
            //add return type
            err = expression(NULL, token, symtab_it, &local_table);
            generate_pop_return();
            free(token->data);//po zjisteni varu nazev smaze
            free_symtab(&local_table, 1);
            return 1000;
         // }
        }
        //no return value
        else{
          printf("\n\nLOCAL----------------------->\n");
          print_sym_tab(&local_table);
          printf("<-------------------------END\n\n\n");
          //add return type
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

  if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("hereReached EOF or Newline where it shouldn't be\n"); return ERROR_SYNTAX;}

  //beginning of function
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
    Token fction_name = *token;

    /*sterv na radku 369 by melo byt bool generate_fnc_begin(char* label);
    label je nazev fce*/

    //sterv bool generate_fnc_begin(char* label);
    generate_fnc_begin(token->data);

    //check if next token is left bracket and not EOF or newline
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); return ERROR_SYNTAX;}

    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeLeftBracket)){
      //check if function with same name already exists
      if(symtab_it_position((char*)token->data, act_table) == NULL){

        fction_name.type = TypeFunc;
        symtab_add_it(act_table, &fction_name); //add name of fction to act_table

        //for easier check of retrun value of params
        int fction_return = fction_params(token, symtab_it_position((char *)fction_name.data, act_table));

        if(fction_return == ERROR_SYNTAX){
          //printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! nastala chyba !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return ERROR_SYNTAX;
        }
        else if(fction_return == ERROR_INTERNAL){
          //printf("\n!!!!!!!!!!!!!!!!!!!!! malloc chyba !!!!!!!!!!!!!!!!!!!\n\n\n");
          return ERROR_INTERNAL;
        }
        else if(fction_return == ERROR_SEMANTIC){
          //printf("chyba semanticka\n");
          return ERROR_SEMANTIC;
        }
        //NO ERROR

        //get new token
        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); return ERROR_SYNTAX;}
        //is there ":"" and EOF after ")"?
        if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)){
          if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
            //konec hlavičky
            //generate???
            if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); return ERROR_SYNTAX;}
            //for easier check of return values form body of function
            int fction_body_return = fction_body(token, symtab_it_position((char *)fction_name.data, act_table));
            //after return check dedent
            //function has return value


            //sterv generate_fnc_end(char* label)

            if (fction_body_return == 0){
              //printf("return\n");
              //newline
              if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); return ERROR_SYNTAX;}

              //check dedent or eof
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                printf("je tady dedent, nebo eof\n");
                generate_fnc_end(fction_name.data);
                free(fction_name.data);//dal uz nebude potreba
                return NO_ERROR;
              }
              //po returnu není dedent
              else{
                printf("chyba kurva1\n");
                return ERROR_SYNTAX;
              }
            }
            //function does not have return value
            else if(fction_body_return == 1000){
              //check dedent or eof
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                printf("je tady dedent, nebo eof\n");
                generate_fnc_end(fction_name.data);
                free(fction_name.data);//dal uz nebude potreba
                return NO_ERROR;
              }
              //po returnu není dedent
              else{
                printf("chyba kurva2\n");
                return ERROR_SYNTAX;
              }
            }
            else if(fction_body_return == ERROR_SEMANTIC){
              //printf("SEMANTICKA CHYBA\n");
              return ERROR_SEMANTIC;
            }
            else if(fction_body_return == ERROR_SYNTAX){
              printf("Syntax CHYBA\n");
              return ERROR_SYNTAX;
            }
            else{ //němelo by se sem dostat
              return ERROR_INTERNAL;
            }
          }
          //po ":" není EOF
          //printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!! CHYBA UKONČENÍ HLAVIČKY EOF!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return ERROR_SYNTAX;
        }
        //po pravé závorce není ":"
        else{
          //printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!! CHYBA UKONČENÍ HLAVIČKY !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
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

  if(GET_TOKEN_CHECK_EOF(&token)){
    DEBUG_PRINT("Test file is empty.\n");
    return 1;
  }

  table = malloc(sizeof(hSymtab));
  symtab_init(table);
  symtab_add_predef_func(table);

  //proc to bylo driv tak slozite?
  generate_main_begin();
  body(&token, table);

  generate_fnc_pre_param();
  generate_push_var("vys", 1);
  int i = 1;
  generate_fnc_param_set_data(TypeInt, &i, 0);
  generate_fnc_call("print");

  generate_main_end();

  //char * str =generator_code_get();
  printf("%s\n", generator_code_get());

/*
  free_symtab(table);
  free(table);
*/
  return err;
}
