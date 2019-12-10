/* stack.h
// název projektu: Implementace překladače imperativního jazyka IFJ19
// tým:            040
// varianta:       II (implementace pomocí tabulky s rozptýlenými položkami)
// autoři:         Boris Burkalo, xburka00, 2BIT
//                 Jiří Herrgott, xherrg00, 2BIT
//                 Jan Klusáček, xklusa14, 2BIT
*/

#ifndef STACK_H
#define STACK_H

#define STACK_TYPE unsigned

typedef struct
{
	STACK_TYPE *content;                    /* pole pro uložení hodnot */
	int top;                                /* index prvku na vrcholu zásobníku */
	unsigned size;                          /* velikost alokovaneho mista */
} Stack;

/* Hlavičky funkcí pro práci se zásobníkem. */

int stack_init(Stack *s);
void stack_free(Stack *s);
int stack_push(Stack *s, STACK_TYPE val);
void stack_pop(Stack *s);
int stack_empty(const Stack *s);

#endif
