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
extern hSymtab *table = NULL;


void assignment(Token *var, Token *value){
  if (GET_TOKEN_CHECK_EOF(value) || TOKEN_TYPE_NEEDED_CHECK(value->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}

  hSymtab_it *tmp_item_var;
  hSymtab_it *tmp_item_value;
  // TODO if the variable has already been added to symtab
  if ((tmp_item_var = symtab_it_position((char*)var->data, table))) {
    if (!(tmp_item_value = symtab_it_position((char*)value->data, table))) {
      if (tmp_item_var->item_type == IT_VAR) {
        if (!(TOKEN_TYPE_NEEDED_CHECK(((hSymtab_Var*)tmp_item_var->data)->type, value->type))) {
          DEBUG_PRINT("SYNTAX ERROR: Variable assigning incorrect value to variable %s.\n", (char*)var->data);
          exit(1);
        }
      }
    }
    else {
      if (tmp_item_value->item_type == IT_VAR) {
        if (!(TOKEN_TYPE_NEEDED_CHECK(((hSymtab_Var*)tmp_item_var->data)->type, ((hSymtab_Var*)tmp_item_value->data)->type))) {
          DEBUG_PRINT("SYNTAX ERROR: Variable assigning incorrect value to variable %s.\n", (char*)var->data);
          exit(1);
        }
      }
      else if (tmp_item_value->item_type == IT_FUNC){
        if (!(TOKEN_TYPE_NEEDED_CHECK(((hSymtab_Var*)tmp_item_var->data)->type, ((hSymtab_Func*)tmp_item_value->data)->return_type))) {
          DEBUG_PRINT("SYNTAX ERROR: Variable assigning incorrect value to variable %s.\n", (char*)var->data);
          exit(1);
        }
      }
    }
  }
  else {
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
        if (tmp_it->item_type == IT_VAR) {
          symtab_add_it(table, var);
          symtab_add_var_data((*table)[symtab_hash_function((char*)var->data)], ((hSymtab_Var*)(tmp_it->data))->type);
        }
        else if (tmp_it->item_type == IT_FUNC) {
          symtab_add_it(table, var);
          symtab_add_var_data((*table)[symtab_hash_function((char*)var->data)], ((hSymtab_Func*)(tmp_it->data))->return_type);
        }
        else {
          DEBUG_PRINT("UNDEFINED ERROR.\n");
          exit(1);
        }

        break;

      default:
        break;

    }
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

  if (strcmp((char*)token->data, "def") == 0) {
    if (fction_start(token) == 0){
      printf("jsem zpátky v body/command\n");
    }
    else{
      printf("chyba\n");
    }
    //tady upravit aby prošlo, když vtupní kód končí definicí
    if (GET_TOKEN_CHECK_EOF(token)); //{DEBUG_PRINT("Reached EOF after function definition\n"); exit(1);}
    body(token);

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
        DEBUG_PRINT("Found EOF, parsing terminated.  %s\n", (char *)token->data);
        return;
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

  while(!TOKEN_TYPE_NEEDED_CHECK(token->type, TypeRightBracket)){

    if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeVariable)){

      check_comma = 0;
      if (malloc_check == false) {

        if( !(((hSymtab_Func *)(symtab_it->data))->params = malloc(sizeof(hSymtab_Func_Param))) ){
          return 99;
        }

        params = ((hSymtab_Func *)(symtab_it->data))->params;
        ((hSymtab_Func *)(symtab_it->data))->params->param_type = TypeUnspecified;

        if( !(((hSymtab_Func *)(symtab_it->data))->params->paramName = malloc(sizeof(char)*strlen((char*)token->data))) ){
          return 99;
        }
        strcpy(params->paramName, (char*)token->data);
        //no need to check name, it is first param

        malloc_check = true;
      }
      //after malloc
      else {
        //for name check
        hSymtab_Func_Param *names = NULL;
        names = ((hSymtab_Func *)(symtab_it->data))->params;

        while(names != NULL){
          //error two parameters with same name
          if(strcmp(names->paramName, (char*)token->data) == 0){
            return 1;
          }
          names = names->next;
        }

        if( !(params->next = malloc(sizeof(hSymtab_Func_Param))) ){
          return 99;
        }
        ((hSymtab_Func *)(symtab_it->data))->params->param_type = TypeUnspecified;

        if( !(params->next->paramName = malloc(sizeof(char)*strlen((char*)token->data))) ){
          return 99;
        }
        strcpy(params->next->paramName, (char*)token->data);

        params = params->next;
      }

    }
    else if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeComma)){
      //printf("comma\n");
      check_comma = 1;
    }
    else{
      return 1;
    }
    if (GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)) {DEBUG_PRINT("Reached EOF or Newline where it shouldn't be\n"); exit(1);}
  }
  if (check_comma == 1){
    //add free
    return 1;
  }
  else{
    return 0;
  }
}

int fction_body(Token *token, hSymtab_it *symtab_it){
  int indent_counter = 0, dedent_counter = 1;

  //create fction_ body, fction command, fction_assignment

  //if there is indent, it is ok
  if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeIndent)){
    indent_counter++;
    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}

    if(strcmp((char*)token->data, "return") == 0){
      return 0;
      //tady budu kontrolovat, jestli něco vracím a pokud jo, tak vrátím číslo
      //a budu vědět, že už mám další token načtený

    }

  }
  else{
    return 1;
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

        fction_name->type = TypeFunc;
        symtab_add_it(table, fction_name); //add name of fction to table

        int fction_return = fction_params(token, symtab_it_position((char *)fction_name->data, table));

        if(fction_return == 1){
          printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! nastala chyba !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return 1;
        }
        else if(fction_return == 99){
          printf("\n!!!!!!!!!!!!!!!!!!!!! malloc chyba !!!!!!!!!!!!!!!!!!!\n\n\n");
          return 99;
        }


        if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}

        //is there ":"" and EOF after ")"?
        if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeColon)){
          if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeNewLine)){
            //konec hlavičky
            //generate???
            if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
            int fction_body_return = fction_body(token, symtab_it_position((char *)fction_name->data, table));
            //after return check dedent
            if (fction_body_return == 0){
              printf("return\n");

              //newline and dedend
              if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}
              //if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF where it shouldn't be\n"); exit(1);}


              //dořešit tady eof
              //if(TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)/* || token == EOF*/){
              if(GET_TOKEN_CHECK_EOF(token) || TOKEN_TYPE_NEEDED_CHECK(token->type, TypeDedend)){
                printf("je tady dedent, nebo eof\n");
                return 0;

              }
              else{
                return 1;
              }
            }
            else if(fction_body_return == 1){
              printf("špatně\n");
              return 1;
            }

            return 0; //?
          }
          printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!! CHYBA UKONČENÍ HLAVIČKY !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
          return 1;
        }
      }
      else{
        return 1; //function already exists
      }
    }
    return 1;
  }
  else{
    return 1; //token is not variable
  }
}
//end of functions
///////////////////////////////////////////////////////////////////////////////////////////////////////

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



//zajistit volání
  if (strcmp((char*)token->data, "def") == 0) {
    if (fction_start(token) == 0){
      printf("jsem zpátky v prog\n");
    }
    if (GET_TOKEN_CHECK_EOF(token)) {DEBUG_PRINT("Reached EOF after function definition\n"); exit(1);}

    body(token);
    //kontrolovat, co to vrátilo, kvůli returnům
    //volat znovu prog, a nebo další definici funkce udělat v body?
    //spíš v body
    return 0;
  }
  else {
    body(token);
  }

  return 0;
}
