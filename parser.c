#include "parser.h"

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

int statement(Token *token, hSymtab *act_table){
  if (!strcmp((char*)token->data, "if")){
    printf("\t \tSENT TO GENERATOR: %s\n", (char*)token->data);
  }
  else if(!strcmp((char*)token->data, "while")) {
    printf("\t \tSENT TO GENERATOR: %s\n", (char*)token->data);
  }

  if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  if (TOKEN_TYPE_NEEDED_CHECK(token, TypeInt) || TOKEN_TYPE_NEEDED_CHECK(token, TypeFloat) || TOKEN_TYPE_NEEDED_CHECK(token, TypeString) ||
      TOKEN_TYPE_NEEDED_CHECK(token, TypeVariable)) {
    err = expression(token, NULL, act_table);

    if (!TOKEN_TYPE_NEEDED_CHECK(token, TypeColon)) {
      DEBUG_PRINT("Syntax error: Colon is missing\n");
      return ERROR_SYNTAX;
    }

    // TODO

  }
  else if (!strcmp((char*)token->data, "None")){

  }

}


int fction_call(Token *token, hSymtab *act_table){
  hSymtab_it *tmp_fct = NULL;
  if ((tmp_fct = symtab_it_position((char*)token->data, act_table))){
    if (tmp_fct->item_type == IT_FUNC) {

    }
  }
}




int assignment(Token *var, Token *value, hSymtab *act_table){

  // GENERATE INSTRUCTION //
    printf("\t \tSENT TO GENERATOR: %s\n", (char*)var->data);
    printf("\t \tSENT TO GENERATOR: %s\n", operNames[value->type]);
  ///

  if (GET_TOKEN_CHECK_EOF(value) || TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  hSymtab_it *tmp_item_var;
  hSymtab_it *tmp_item_value;

    if (!symtab_it_position((char*)var->data, act_table))
      symtab_add_it(act_table, var);
    err = expression(value, (*act_table)[symtab_hash_function((char*)var->data)], act_table);
    if (err == ERROR_SYNTAX || err == ERROR_SEMANTIC) {
      return err;
    }
    ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->defined = true;
    ((hSymtab_Var*)((*act_table)[symtab_hash_function((char*)var->data)]->data))->global = true;

    return err;

}


int command(Token *token, hSymtab *act_table, int in_function){

  if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)) {
    Token token_n;

    if (GET_TOKEN_CHECK_EOF(&token_n)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}



    if (TOKEN_TYPE_NEEDED_CHECK(token_n.type, TypeAssignment)) {

      assignment(token, &token_n, act_table);
      if (in_function == 1 && !symtab_it_position((char*)token->data, table)) {
        ((hSymtab_Var*)symtab_it_position((char*)token->data, act_table)->data)->global = false;
      }
      return err;


    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token_n.type, TypeLeftBracket)){

      // funguje pouze s globalni tabulkou // TODO
      fction_call(token, act_table);
    }
  }
  else if (strcmp((char*)token->data, "def") == 0) {
    free(token->data);//po tomhle ifu useless
    if (fction_start(token, act_table) == 0){
      printf("jsem zpátky v body/command\n\n\n");
    }
    else{
      printf("chyba\n");
    }
    //tady upravit aby prošlo, když vstupní kód končí definicí
    if (in_function == 1){
      return;
    }
    else{
      if (GET_TOKEN_CHECK_EOF(token)){DEBUG_PRINT("Reached EOF after function definition\n"); return 0;}//exit(1);}
      body(token, act_table);
    }

  } // Pridat None a Pass
  else if (TOKEN_TYPE_NEEDED_CHECK(token->type, TypeKeyword)) {
      err = statement(token, act_table);
  }
  return false;
}

int body(Token *token, hSymtab *act_table){
  while (1) {
      if (err == NO_ERROR) {
        command(token, act_table, 0);
        if (err == ERROR_SYNTAX || err == ERROR_SEMANTIC) {
          DEBUG_PRINT("Error occured, parsing ended.\n");
          return err;
        }
      }
      else{
        DEBUG_PRINT("Found EOF, parsing terminated.\n");
        return err;
      }
      if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return;}

  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//functions

int fction_params(Token *token, hSymtab_it *symtab_it){
  //need new token, now token is left bracket
  if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
  int check_comma = 0; //params should not end with comma
  bool malloc_check = false; //we want malloc only fist time
  hSymtab_Func_Param *params = NULL;
  unsigned param_counter = 0;

  while(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)){
    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
      param_counter++;
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
            return 1;
          }
          if (!names->next) {
            break;
          }
            names = names->next;
        }

        //printf("%s\n", (char*)token->data);

        if( !(params->next = malloc(sizeof(hSymtab_Func_Param))) ){
          return 99;
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
        params = params->next;
        params->next = NULL;
      }

    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeComma)){
      //printf("%d\n", token->type);
      check_comma = 1;
    }
    else{
      //param není variable nebo comma
      return 1;
    }
    //printf("%d\n", token->type);
    //get next token
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
    //printf("%d\n", token->type);
  }
  if (check_comma == 1){
    //add free
    //parametry končí čárkou
    return ERROR_SYNTAX;
  }
  else{
    //parametry vypadají v pořádku
    return 0;
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
    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}

      while (strcmp((char*)token->data, "return") != 0) {
          if (err == NO_ERROR) {
            command(token, &local_table, 1);
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
          if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Found EOF, parsing terminated.\n"); return 1;}

      }
      free(token->data);//smaze "return"
      if(indent_counter == dedent_counter){
        //doplnit return type

        //code ends with definition
        if(GET_TOKEN_CHECK_EOF(token)) return 0;
        //return value is variable or something

        if(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
          //variable
          if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
            if(symtab_it_position((char*)token->data, &local_table) == NULL){
              return 1; //return variable is not in local_table of function
            }
            else{

              printf("\n\nLOCAL----------------------->\n");
              print_sym_tab(&local_table);
              printf("<-------------------------END\n\n\n");
              //add return type
              err = expression(token, symtab_it, &local_table);
              free(token->data);//po zjisteni varu nazev smaze
              free_symtab(&local_table, 1);
              return 1000;
            }
          }
          //something else, need fix for only possible return types
          //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          else{
            printf("\n\nLOCAL----------------------->\n");
            print_sym_tab(&local_table);
            printf("<-------------------------END\n\n\n");
            //add return type
            err = expression(token, symtab_it, &local_table);
            free(token->data);//po zjisteni varu nazev smaze
            free_symtab(&local_table, 1);
            return 1000;
          }
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
        free_symtab(&local_table, 1);
        return 1;
      }

      //return 0;
      //return 1;
    //}

  }
  else{
    return 1;
  }
}

int fction_start(Token *token, hSymtab *act_table){
  //return 1 = error
  if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  //beginning of function
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){
    Token fction_name = *token;

    /*sterv na radku 369 by melo byt bool generate_fnc_begin(char* label);
    label je nazev fce*/

    //sterv bool generate_fnc_begin(char* label);

    //check if next token is left bracket and not EOF or newline
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeLeftBracket)){
      //check if function with same name already exists
      if(symtab_it_position((char*)token->data, act_table) == NULL){

        fction_name.type = TypeFunc;
        symtab_add_it(act_table, &fction_name); //add name of fction to act_table

        //for easier check of retrun value of params
        int fction_return = fction_params(token, symtab_it_position((char *)fction_name.data, act_table));

        if(fction_return == ERROR_SYNTAX){
          printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! nastala chyba !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return 1;
        }
        else if(fction_return == 99){
          printf("\n!!!!!!!!!!!!!!!!!!!!! malloc chyba !!!!!!!!!!!!!!!!!!!\n\n\n");
          return 99;
        }
        //get new token
        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
        //is there ":"" and EOF after ")"?
        if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)){
          if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
            //konec hlavičky
            //generate???
            if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
            //for easier check of return values form body of function
            int fction_body_return = fction_body(token, symtab_it_position((char *)fction_name.data, act_table));
            //after return check dedent
            //function has return value
            free(fction_name.data);//dal uz nebude potreba

            //sterv generate_fnc_end(char* label)

            if (fction_body_return == 0){
              printf("return\n");
              //newline
              if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}

              //check dedent or eof
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                printf("je tady dedent, nebo eof\n");
                return 0;
              }
              //po returnu není dedent
              else{
                printf("chyba kurva\n");
                return 1;
              }
            }
            //function does not have return value
            else if(fction_body_return == 1000){
              //check dedent or eof
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                printf("je tady dedent, nebo eof\n");
                return 0;
              }
              //po returnu není dedent
              else{
                printf("chyba kurva\n");
                return 1;
              }
            }
            else if(fction_body_return == 1){
              printf("špatně\n");
              return 1;
            }
            //jiná návratová hodnota než 1 nebo 0
            return 1;
          }
          //po ":" není EOF
          printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!! CHYBA UKONČENÍ HLAVIČKY !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return 1;
        }
        //po pravé závorce není ":"
        else{
          printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!! CHYBA UKONČENÍ HLAVIČKY !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return 1;
        }
      }
      else{
        return 1; //function already exists
      }
    }
    //po názvu funkce nenásleduje "("
    return 1;
  }
  //po def není TypeVariable
  else{
    return 1;
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

  if(GET_TOKEN_CHECK_EOF(&token)){
    DEBUG_PRINT("Test file is empty.\n");
    return 1;
  }

  table = malloc(sizeof(hSymtab));
  symtab_init(table);
  symtab_add_predef_func(table);

  //proc to bylo driv tak slozite?
  body(&token, table);

/*
  free_symtab(table);
  free(table);
*/
  return err;
}
