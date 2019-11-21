#include <stdio.h>
#include <stdlib.h>

//#include "sym_tab.h"
//#include "scanner.h"
#include "parser.c"

hSymtab *table;

extern const char *operNames[] = {"+", "-", "*", "/", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "new line", "keyword", "variable", "string", "int", "float", "indent", "dedent"};


void print_sym_tab(hSymtab *table){
  DEBUG_PRINT("PRINTING WHOLE TABLE\n\n");
  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*table)[i]) {
      printf("-------------------------------------------------------------------------------\n");
      if ((*table)[i]->item_type == IT_VAR)
        printf("VARIABLE: %-10s \t  %-5s %s\n", (*table)[i]->hKey, "|  TYPE:", operNames[((hSymtab_Var*)((*table)[i]->data))->type]);
      else if ((*table)[i]->item_type == IT_FUNC) {
        printf("FUNCTION: %-10s \t  %-5s %s\n", (*table)[i]->hKey, "|  RETURN TYPE:",operNames[((hSymtab_Func*)((*table)[i]->data))->return_type]);
        hSymtab_Func_Param *paramets = ((hSymtab_Func*)((*table)[i]->data))->params;
        if (!paramets)
          printf("%-25s -> FUNCTION DOESN'T HAVE ANY PARAMETERS\n", " ");
        else {
          for (int i = 0; paramets; i++){
              printf("%-25s -> PARAMETER nr.%d --> TYPE: %-10s  NAME: %s\n", " ", i, operNames[paramets->param_type], paramets->paramName == NULL ? "NULL" : paramets->paramName);
              paramets = paramets->next;
          }
        }
      }
    }
  }
}

int main() {
  prog();
  print_sym_tab(table);
  printf("\n");
  DEBUG_PRINT("_____PARSING ENDED_____\n");
}
