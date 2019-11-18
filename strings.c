#include "strings.h"
#include <malloc.h>

#define STR_SIZE 8

static inline size_t align_size(size_t size)
{
    return (size % (STR_SIZE)) != 0 ? ( (STR_SIZE) - ( size % (STR_SIZE) ) ) + size : size;
}

int str_init(String *str)
{
	if ((str->content = malloc(sizeof(char) * STR_SIZE)) == NULL)
		return 0;

	str->size = STR_SIZE;
	str->asize = 0;
	str->content[str->asize] = '\0';
	return 1;
}

int str_pushc(String *str, char new_char)
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

int str_push(String *str, char *new_str)
{
	if (str->size < str->asize)
		return 0;

	size_t str_size = strlen(new_str);

	if (str->size < str->asize + 1 + str_size)
	{
		size_t new_size = align_size(str_size + 1);
		if ((str->content = realloc(str->content, str->size + new_size)) == NULL)
			return 0;
		str->size += new_size;
	}
	strcat(str->content, new_str);
	str->asize += str_size;
	return 1;
}

char str_pop(String *str)
{
	str->asize--;
	char c = str->content[str->asize];
	str->content[str->asize] = '\0';
	return c;
}

void str_clear(String *str)
{
	str->asize = 0;
	str->content[str->asize] = '\0';
}

void str_free(String *str)
{
	free(str->content);
	str->content = NULL;
	str->asize = 0;
	str->size = 0;
}
