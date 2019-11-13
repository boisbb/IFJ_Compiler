#include "sym_tab.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h> //uint32_t

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

hSymtab_it *symtab_it_position(char *searched_for, hSymtab *sym_tab){
  hSymtab_it *tmp = (*sym_tab)[symtab_hash_function(searched_for)];
  while (tmp != NULL) {
    if(!(strcmp(searched_for, tmp->hKey))) return tmp;
    tmp = tmp->next;
  }
  return NULL;
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
    (*sym_tab)[hash]->data = NULL;
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
    tmp->data = NULL;
  }
}

void symtab_add_value(hSymtab *sym_tab, Token *token_var,Token *token_val){
  int hash = symtab_hash_function((char*)token_var->data);
  hSymtab_it *tmp = symtab_it_position((char*)token_var->data, sym_tab);
  if (tmp == NULL) {
    DEBUG_PRINT("SYNTAX ERROR: The variable: %s couldn't be found\n", (char*)token_var->data);
    exit(1);
  }

  if ((tmp->data = malloc(sizeof(hSymtab_data))) == NULL) {
    DEBUG_PRINT("Memory allocation for sym_tab_it->data failed.");
    exit(1);
  }


  if (token_val->type == TypeInt) {
    tmp->data->type = TypeInt;
    tmp->data->value_int = *(int*)token_val->data;
    // SOMEHOW solve global variables TODO
  }
  else if (token_val->type == TypeVariable) {
    hSymtab_it *tmp_val = NULL;
    if ((tmp_val = symtab_it_position((char*)token_val->data, sym_tab)) == NULL) {
      DEBUG_PRINT("SYNTAX ERROR: The variable: %s couldn't be found\n", (char*)token_val->data);
      exit(1);
    }

    tmp->data->type = tmp_val->data->type;
    if (tmp_val->data->type == TypeInt) {
      tmp->data->value_int = tmp_val->data->value_int;
    }
    else if (tmp_val->data->type == TypeString) {
      tmp->data->value_str = malloc(sizeof(char) * strlen(tmp_val->data->value_str));
      strcpy(tmp->data->value_str, tmp_val->data->value_str);
      // SOMEHOW solve global variables TODO
    }
    else if (tmp_val->data->type == TypeFloat) {
      tmp->data->value_float = tmp_val->data->value_float;
      // SOMEHOW solve global variables TODO
    }
  }
  else if(token_val->type == TypeString) {
    tmp->data->type = TypeString;
    if ((tmp->data->value_str = malloc(sizeof(char) * strlen((char*)token_val->data))) == NULL) {
      DEBUG_PRINT("Memory allocation for sym_tab_it->data failed.");
      exit(1);
    }
    strcpy(tmp->data->value_str, (char*)token_val->data);
  }
}
