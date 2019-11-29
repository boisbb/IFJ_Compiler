#include "sym_tab.h"
#include <stdint.h>
// HASHING FUNCTION //
unsigned int symtab_hash_function(const char *str) {
  uint32_t h=0;     // musí mít 32 bitů
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

// To be deleted
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
  //printf("%d\n", tmp->hKey == NULL);
  while (tmp != NULL) {
    if(!(strcmp(searched_for, tmp->hKey))) return tmp;
    tmp = tmp->next;
  }
  return NULL;
}

int is_item_var_defined(char *desired, hSymtab *sym_tab){
  if (!(symtab_it_position(desired, sym_tab))) {
    DEBUG_PRINT("ERROR: variable %s does not existabc.\n", desired);
    return ERROR_SEMANTIC;
  }
  else {
    if ((symtab_it_position(desired, sym_tab)->item_type) == IT_VAR) {
      if (((hSymtab_Var*)(symtab_it_position(desired, sym_tab)->data))->defined == false) {
        return ERROR_SEMANTIC;
      }
    }
  }

  return NO_ERROR;
}


int symtab_add_it(hSymtab *sym_tab, Token *token){
  int hash = symtab_hash_function((char*) token->data);
  if ((*sym_tab)[hash] == NULL) {
    if (((*sym_tab)[hash] = malloc(sizeof(hSymtab_it))) == NULL
    || ((*sym_tab)[hash]->hKey = malloc(sizeof(char) * (strlen((char*)token->data) + 1))) == NULL) {

      return ERROR_INTERNAL;
    }
    (*sym_tab)[hash]->next = NULL;
    strcpy((*sym_tab)[hash]->hKey, (char *)token->data);
  }
  else {
    hSymtab_it *tmp = (*sym_tab)[hash];

    find_last_in_list(tmp);
    if ((tmp = malloc(sizeof(hSymtab_it))) == NULL) {
      return ERROR_INTERNAL;
    }
    tmp->next = NULL;
    strcpy(tmp->hKey, (char*)token->data);
  }
  switch(token->type) {

    case TypeVariable:
      if (!((*sym_tab)[hash]->data = malloc(sizeof(hSymtab_Var)))){
        return ERROR_INTERNAL;
      }
      (*sym_tab)[hash]->item_type = IT_VAR;
      ((hSymtab_Var*)((*sym_tab)[hash]->data))->global = -1;
      ((hSymtab_Var*)((*sym_tab)[hash]->data))->type = TypeUnspecified;
      ((hSymtab_Var*)((*sym_tab)[hash]->data))->defined = false;
      break;


    case TypeFunc:
      if (!((*sym_tab)[hash]->data = malloc(sizeof(hSymtab_Func)))){
        return ERROR_INTERNAL;
      }
      (*sym_tab)[hash]->item_type = IT_FUNC;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->defined = false;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->params = NULL;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->paramNum = 0;
      ((hSymtab_Func*)((*sym_tab)[hash]->data))->return_type = TypeUndefined;
      break;

    default:
      return ERROR_SYNTAX;
  }

  return NO_ERROR;
}

int symtab_add_var_data(hSymtab_it *symtab_it, Type type){
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
      return ERROR_SYNTAX;
  }
  return NO_ERROR;
}

int symtab_add_predef_func(hSymtab *table){
  Token tok_fc;

  // print(str,str,str,....) - set only for one, further check of arguments required in parser
  /*if (!(tok_fc.data = malloc(sizeof(char) * (strlen("print") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  strcpy((char*)tok_fc.data, "print");*/
  tok_fc.data = "print";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;
  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = calloc(1, sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->paramName = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeUndefined;
  //free(tok_fc.data);
  //

  // inputs()
  //if (!(tok_fc.data = malloc(sizeof(char) * (strlen("inputs") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  //strcpy((char*)tok_fc.data, "inputs");
  tok_fc.data = "inputs";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeString;
  //free(tok_fc.data);
  //

  // inputf()
  //if (!(tok_fc.data = malloc(sizeof(char) * (strlen("inputf") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  //strcpy((char*)tok_fc.data, "inputf");
  tok_fc.data = "inputf";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeFloat;
  //free(tok_fc.data);
  //

  // inputi()
  //if (!(tok_fc.data = malloc(sizeof(char) * (strlen("inputi") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  //strcpy((char*)tok_fc.data, "inputi");
  tok_fc.data = "inputi";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeInt;
  //free(tok_fc.data);
  //

  // len(str) //
  //if (!(tok_fc.data = malloc(sizeof(char) * (strlen("len") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  //strcpy((char*)tok_fc.data, "len");
  tok_fc.data = "len";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = calloc(1, sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->paramName = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->paramNum = 1;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeInt;
  //free(tok_fc.data);
  //

  // substr(str,int,int) //
  //if (!(tok_fc.data = malloc(sizeof(char) * (strlen("substr") + 1)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  //strcpy((char*)tok_fc.data, "substr");
  tok_fc.data = "substr";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = malloc(sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->paramName = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->paramNum = 3;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeString;
  hSymtab_Func_Param *f_param = ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params;

  for (int i = 0; i < 2; i++) {
    if(!(f_param->next = calloc(1, sizeof(hSymtab_Func_Param)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;}
    f_param->next->param_type = TypeInt;
    f_param->next->paramName = NULL;
    f_param = f_param->next;
  }
  //free(tok_fc.data);
  //

  // ord(str,int) //
  //if (!(tok_fc.data = malloc(sizeof(char) * (strlen("ord") + 1)))) {DEBUG_PRINT("Failed to allocate memory for function.\n"); return;}
  //strcpy((char*)tok_fc.data, "ord");
  tok_fc.data = "ord";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = malloc(sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeString;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->paramName = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->paramNum = 2;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeInt;
  f_param = ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params;

  for (int i = 0; i < 1; i++) {
    if(!(f_param->next = calloc(1, sizeof(hSymtab_Func_Param)))){DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;}
    f_param->next->param_type = TypeInt;
    f_param->next->paramName = NULL;
    f_param = f_param->next;
  }
  //free(tok_fc.data);
  //

  // chr(int) //
  tok_fc.data = "chr";
  tok_fc.type = TypeFunc;
  symtab_add_it(table, &tok_fc);
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->defined = true;

  if (!(((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params = calloc(1, sizeof(hSymtab_Func_Param)))) {
    DEBUG_PRINT("Failed to allocate memory for function.\n"); return ERROR_INTERNAL;
  }

  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->param_type = TypeInt;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->params->paramName = NULL;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->paramNum = 1;
  ((hSymtab_Func*)((*table)[symtab_hash_function((char*)tok_fc.data)]->data))->return_type = TypeString;
  //free(tok_fc.data);
  //

  return NO_ERROR;
}

int symtab_copy(hSymtab *origin, hSymtab *new){

  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*origin)[i] != NULL) {
      (*new)[i] = (*origin)[i];
    }
  }
  return NO_ERROR;
}

void free_symtab(hSymtab *table, int switch_local){
  hSymtab_it *tmp;
  hSymtab_it *swp;
  hSymtab_Func_Param *tmp_param = NULL;
  hSymtab_Func_Param *swp_param = NULL;

  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*table)[i] != NULL) {
      tmp = (*table)[i];
      while (tmp) {
        swp = tmp->next;
        if (switch_local) {
          if (tmp->item_type == IT_FUNC) {
            tmp = swp;
            continue;
          }
          else if (tmp->item_type == IT_VAR){
            if (((hSymtab_Var*)tmp->data)->global == 1) {
              tmp = swp;
              continue;
            }
          }
        }
        if (tmp->item_type == IT_FUNC) {
          if (((hSymtab_Func*)tmp->data)->params){
            tmp_param = ((hSymtab_Func*)tmp->data)->params;
            while (tmp_param) {
              swp_param = tmp_param->next;
              free(tmp_param->paramName);
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



hSym_fct_node *sym_stack_push(hSym_fct_node* f_top, char *name){
  if (f_top->fct_name == NULL) {
    //printf("a\n");
      //printf("a\n");
      f_top->fct_name = name;
      f_top->param_num = 0;
      f_top->next = NULL;
      f_top->prev = NULL;
      return f_top;
  }
  if (!(f_top->next = malloc(sizeof(hSym_fct_node))) /*|| !(stack.top->next->prev = malloc(sizeof(TermStackIt)))*/) {
    DEBUG_PRINT("INTERNAL ERROR: Memory allocation failed.\n");
    return NULL;
  }

  f_top->next->prev = f_top;
  f_top->next->fct_name = name;
  f_top->next->param_num = 0;
  f_top->next->next = NULL;
  f_top = f_top->next;
  return f_top;
  //printf("%s\n", f_top->fct_name);

  return f_top;

}

int sym_stack_pop_all(hSym_fct_node* f_top, hSymtab* act_table){
  if (!f_top->fct_name) {
    return NO_ERROR;
  }

  hSym_fct_node* tmp_top = f_top;
  hSym_fct_node* swp;
  hSymtab_it* fction_item;
  while (tmp_top) {
    fction_item = symtab_it_position(tmp_top->fct_name, act_table);
    if (!fction_item) {
      DEBUG_PRINT("Semantic Error: function %s was not defined.\n", tmp_top->fct_name);
      return ERROR_SEMANTIC;
    }
    else{
      if (fction_item->item_type == IT_FUNC) {
        if (((hSymtab_Func*)fction_item->data)->paramNum == tmp_top->param_num){
          swp = tmp_top->prev;
          free(tmp_top->fct_name);
          free(tmp_top);
          tmp_top = swp;
        }
        else {
          //DEBUG_PRINT("Semantic Error: function %s was not defined.\n", tmp_top->fct_name);
          return ERROR_SEMANTIC_FUNCTION_CALL;
        }
      }
      else {
        DEBUG_PRINT("Semantic Error: function %s was not defined.\n", tmp_top->fct_name);
        return ERROR_SEMANTIC;
      }
    }
  }

  return NO_ERROR;
}
