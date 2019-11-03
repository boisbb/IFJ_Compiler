#include <malloc.h>
#include "strings.h"

#define STR_SIZE 8

int str_init(String *str)
{
	if ((str->content = malloc(sizeof(char) * STR_SIZE)) == NULL)
		return 0;

	str->size = STR_SIZE;
	str->asize = 0;
	str->content[str->asize] = '\0';
	return 1;
}

int str_add(String *str, char new_char)
{
	if (str->size < str->asize)
		return 0;

	if (str->size == str->asize + 1)
	{
		if ((str->content = realloc(str->content, str->size + STR_SIZE)) == NULL)
			return 0;
		str->size += STR_SIZE;
	}
	str->content[str->asize] = new_char;
	str->asize++;
	str->content[str->asize] = '\0';
	return 1;
}

char str_pop(String *str)
{
	str->asize--;
	char c = str->content[str->asize];
	str->content[str->asize] = '\0';
	return c;
}

void str_free(String *str)
{
	free(str->content);
	str->content = NULL;
	str->asize = 0;
	str->size = 0;
}
