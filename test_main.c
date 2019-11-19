#include <stdio.h>
#include <stdlib.h>

//#include "sym_tab.h"
//#include "scanner.h"
#include "parser.c"

hSymtab *table;

void print_sym_tab(hSymtab *table){
  DEBUG_PRINT("PRINTING WHOLE TABLE\n\n");
  for (int i = 0; i < HTAB_PRIME; i++){
    if ((*table)[i]) {
      printf("%s %d\n", (*table)[i]->hKey, (*table)[i]->item_type);
    }
  }
}


int main() {
  prog();

  print_sym_tab(table);
  printf("\n");
  DEBUG_PRINT("_____PARSING ENDED_____\n");

}
