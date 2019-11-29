#include <stdio.h>
#include <stdlib.h>

//#include "sym_tab.h"
//#include "scanner.h"
#include "parser.h"
#include "generator.h"

hSymtab *table;

const char *operNames[] = {"+", "-", "*", "/", "//", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "new line", "keyword", "variable", "string", "documentary string", "int", "float", "indent", "dedent", "None", "pass", "unspecified", "func", "undefined"};


void print_sym_tab(hSymtab *table){
  DEBUG_PRINT("PRINTING WHOLE TABLE\n\n");
  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*table)[i]) {
      fprintf(stderr,"-------------------------------------------------------------------------------\n");
      if ((*table)[i]->item_type == IT_VAR)
        fprintf(stderr,"VARIABLE: %-10s \t  %-5s %s\n", (*table)[i]->hKey, "|  TYPE:", operNames[((hSymtab_Var*)((*table)[i]->data))->type]);
      else if ((*table)[i]->item_type == IT_FUNC) {
        fprintf(stderr,"FUNCTION: %-10s \t  %-5s %s\n", (*table)[i]->hKey, "|  RETURN TYPE:",operNames[((hSymtab_Func*)((*table)[i]->data))->return_type]);
        hSymtab_Func_Param *paramets = ((hSymtab_Func*)((*table)[i]->data))->params;
        if (!paramets)
          fprintf(stderr,"%-25s -> FUNCTION DOESN'T HAVE ANY PARAMETERS\n", " ");
        else {
          for (int i = 0; paramets; i++){
              fprintf(stderr,"%-25s -> PARAMETER nr.%d --> TYPE: %-10s  NAME: %s\n", " ", i, operNames[paramets->param_type], paramets->paramName == NULL ? "NULL" : paramets->paramName);
              paramets = paramets->next;
          }
        }
      }
    }
  }
}

int main() {
  int err = -1;
  err = prog();
  if (err != NO_ERROR) {
    //DEBUG_PRINT("_____PARSING ENDED WITH ERROR %d_____\n", err);
    return err;
  }
  //fprintf(stderr, "a\n");
  //print_sym_tab(table);

  if (err == NO_ERROR){
    printf("%s\n", generator_code_get());
  }
  generator_free();

  //fprintf(stderr,"\n");
  //DEBUG_PRINT("_____PARSING ENDED SUCCESSFULLY_____\n");
  free_symtab(table, 0);
  free(table);

  scanner_free();
}
