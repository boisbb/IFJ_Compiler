#include <stdio.h>
#include <stdlib.h>

//#include "sym_tab.h"
//#include "scanner.h"
#include "parser.c"

hSymtab *table;
/*
void print_sym_tab(hSymtab *table){
  DEBUG_PRINT("PRINTING WHOLE TABLE\n\n");
  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*table)[i] != NULL) {
      if ((*table)[i]->data->type == TypeInt)
        printf("\t |Variable: %s| \t |Value: %d| \t |Type: INT|\n", (*table)[i]->hKey, (*table)[i]->data->value_int);
      else if ((*table)[i]->data->type == TypeString)
        printf("\t |Variable: %s| \t |Value: %s| \t |Type: STRING|\n", (*table)[i]->hKey, (*table)[i]->data->value_str);
      else if ((*table)[i]->data->type == TypeFloat)
        printf("\t |Variable: %s| \t |Value: %f| \t |Type: FLOAT|\n", (*table)[i]->hKey, (*table)[i]->data->value_float);
    }
  }
}
*/

int main() {
  prog();
  //print_sym_tab(table);
  //printf("\n");
  //DEBUG_PRINT("_____PARSING ENDED_____\n");
}
