/* stack.c
// název projektu: Implementace překladače imperativního jazyka IFJ19
// tým:            040
// varianta:       II (implementace pomocí tabulky s rozptýlenými položkami)
// autoři:         Boris Burkalo, xburka00, 2BIT
//                 Jiří Herrgott, xherrg00, 2BIT
//                 Jan Klusáček, xklusa14, 2BIT
*/

#include "stack.h"
#include <malloc.h>

#define STACK_SIZE 20

int stack_init(Stack* s)
{
	if(s == NULL)
		return 0;

	if ((s->content = malloc(sizeof(STACK_TYPE) * STACK_SIZE)) == NULL)
		return 0;

	s->top = -1;
	s->size = STACK_SIZE;
	return 1;
}

void stack_free(Stack *s)
{
	free(s->content);
	s->size = 0;
	s->top = -1;
	s->content = NULL;
}

int stack_push(Stack* s, STACK_TYPE val)
{
	if(s->top + 1 == (int)s->size)
	{
		if ((s->content = realloc(s->content, s->size + STACK_SIZE)) == NULL)
			return 0;
		s->size += STACK_SIZE;
	}

	s->top++;
	s->content[s->top] = val;
	return 1;
}

void stack_pop(Stack* s)
{
	if(!stack_empty(s))
		s->top--;
}

int stack_empty(const Stack* s)
{
	return s->top == -1 ? 1 : 0;
}
