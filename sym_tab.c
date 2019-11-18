#include "sym_tab.h"
#include "error.h"

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
    || ((*sym_tab)[hash]->hKey = malloc(sizeof(char) * strlen((char*)token->data))) == NULL) {

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

void symtab_add_value(hSymtab *sym_tab, hSymtab_it *symtab_it,Token *token_val){
  // TODO
}
