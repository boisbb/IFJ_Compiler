#include "sym_tab.h"


// HASHING FUNCTION //
unsigned int symtab_hash_function(const char *str) {
  __uint32_t h=0;     // musí mít 32 bitů
  const unsigned char *p;
  for(p=(const unsigned char*)str; *p!='\0'; p++)
      h = 65599*h + *p;
  return h % HTAB_PRIME;
}

void symtab_init(hSymtab *sym_tab){
  for (int i = 0; i < HTAB_PRIME; i++) {
    (*sym_tab)[i] = NULL;
  }
}

void find_last_in_list(hSymtab_it *sym_tab_it){
  while (sym_tab_it != NULL) {
    sym_tab_it = sym_tab_it->next;
  }
}

int symtab_num_of_fction_params(hSymtab_Func* symtab_func_data){
  int num_of_params = 0;
  hSymtab_Func_Param *parameter = symtab_func_data->params;
  while(parameter){
    num_of_params++;
    parameter = parameter->next;
  }
  return num_of_params;
}

hSymtab_it *symtab_it_position(char *searched_for, hSymtab *sym_tab){
  hSymtab_it *tmp = (*sym_tab)[symtab_hash_function(searched_for)];
  while (tmp != NULL) {
    if(!(strcmp(searched_for, tmp->hKey))) return tmp;
    tmp = tmp->next;
  }
  return NULL;
}

int symtab_it_get_type(hSymtab_it *symtab_it){
  /*if (symtab_it->data->type == TypeInt) {
    return TypeInt;
  }
  else if (symtab_it->data->type == TypeFloat) {
    return TypeFloat;
  }
  else if (symtab_it->data->type == TypeString) {
    return TypeString;
  }*/
  return 0;
}

void symtab_add_it(hSymtab *sym_tab, Token *token){
  int hash = symtab_hash_function((char*) token->data);
  if ((*sym_tab)[hash] == NULL) {
    if (((*sym_tab)[hash] = malloc(sizeof(hSymtab_it))) == NULL
    || ((*sym_tab)[hash]->hKey = malloc(sizeof(char) * (strlen((char*)token->data) + 1))) == NULL) {

      DEBUG_PRINT("Memory allocation for symbol table faield.");
      exit(1);
    }
    (*sym_tab)[hash]->next = NULL;
    strcpy((*sym_tab)[hash]->hKey, (char *)token->data);
  }
  else {
    hSymtab_it *tmp = (*sym_tab)[hash];

    find_last_in_list(tmp);
    if ((tmp = malloc(sizeof(hSymtab_it))) == NULL) {
      DEBUG_PRINT("Memory allocation for symbol table faield.");
      exit(1);
    }
    tmp->next = NULL;
    strcpy(tmp->hKey, (char*)token->data);
  }

  switch(token->type) {

    case TypeVariable:
      (*sym_tab)[hash]->data = malloc(sizeof(hSymtab_Var));
      (*sym_tab)[hash]->item_type = IT_VAR;
      ((hSymtab_Var*)((*sym_tab)[hash]->data))->global = -1;
      ((hSymtab_Var*)((*sym_tab)[hash]->data))->type = TypeUnspecified;
      ((hSymtab_Var*)((*sym_tab)[hash]->data))->defined = false;
      break;


    case TypeFunc:
      (*sym_tab)[hash]->data = malloc(sizeof(hSymtab_Func));
      (*sym_tab)[hash]->item_type = IT_FUNC;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->defined = false;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->params = NULL;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->return_type = TypeUnspecified;
      break;

    default:
      break;
  }
}

void symtab_add_var_data(hSymtab_it *symtab_it, Type type){
  switch(type){
    case TypeInt:
      ((hSymtab_Var*)(symtab_it->data))->type = TypeInt;
      break;
    case TypeString:
      ((hSymtab_Var*)(symtab_it->data))->type = TypeString;
      break;
    case TypeFloat:
      ((hSymtab_Var*)(symtab_it->data))->type = TypeFloat;
      break;
    default:
      break;
  }
}

void symtab_add_predef_func(hSymtab *table){
  Token tok_fc;

  // print(str,str,str,....) - set only for one, further check of arguments required in parser
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("print") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "print");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;
  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = calloc(1, sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  free(tok_fc.data);
  //

  // inputs()
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("inputs") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "inputs");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeString;
  free(tok_fc.data);
  //

  // inputf()
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("inputf") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "inputf");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeFloat;
  free(tok_fc.data);
  //

  // inputi()
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("inputi") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "inputi");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeInt;
  free(tok_fc.data);
  //

  // len(str) //
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("len") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "len");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = calloc(1, sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeInt;
  free(tok_fc.data);
  //

  // substr(str,int,int) //
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("substr") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "substr");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = malloc(sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeString;
  hSymtab_Func_Param *f_param = ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params;

  for (int i = 0; i < 2; i++) {
    if(!(f_param->next = calloc(1, sizeof(hSymtab_Func_Param)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
    f_param->next->param_type = TypeInt;
    f_param = f_param->next;
  }
  free(tok_fc.data);
  //

  // ord(str,int) //
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("ord") + 1)))) {DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "ord");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = malloc(sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeInt;
  f_param = ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params;

  for (int i = 0; i < 1; i++) {
    if(!(f_param->next = calloc(1, sizeof(hSymtab_Func_Param)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
    f_param->next->param_type = TypeInt;
    f_param = f_param->next;
  }
  free(tok_fc.data);
  //

  // chr(int) //
  if (!(tok_fc.data = malloc(sizeof(char) * (strlen("chr") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "chr");

  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = calloc(1, sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeInt;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeString;
  free(tok_fc.data);
  //

}

void free_symtab(hSymtab *table){
  hSymtab_it *tmp;
  hSymtab_it *swp;
  hSymtab_Func_Param *tmp_param = NULL;
  hSymtab_Func_Param *swp_param = NULL;

  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*table)[i] != NULL) {
      tmp = (*table)[i];
      while (tmp) {
        swp = tmp->next;
        if (tmp->item_type == IT_FUNC) {
          if (((hSymtab_Func*)tmp->data)->params){
            tmp_param = ((hSymtab_Func*)tmp->data)->params;
            while (tmp_param) {
              swp_param = tmp_param->next;
              free(tmp_param);
              tmp_param = swp_param;
            }
          }
        }
        free(tmp->hKey);
        free(tmp->data);
        free(tmp);
        tmp = swp;
      }
    }
  }
}
