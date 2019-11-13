#include "scanner.h"

#include <stdio.h>//fgetc, stdin, EOF, ungetc
#include <stdlib.h>//malloc, strtol, strtod
#include <stdbool.h>
#include <ctype.h>//isalpha, isalnum, isdigit, isxdigit
#include <string.h>//strcpy, strcmp
#include <limits.h>//INT_MAX

#include "error.h"
#include "strings.h"
#include "stack.h"


/////// CONSTANTS ////
const char *keywords[] = {"def", "else", "if", "None", "pass", "return", "while"};
//////////////////////


/////// STATES ///////
#define STATE_INITIAL 259

#define STATE_EQUALS 260
#define STATE_GREATER 261
#define STATE_LESSER 262
#define STATE_NEGATION 263

#define STATE_INDENTATION 267

#define STATE_VARIABLE 268

#define STATE_STRING 269
#define STATE_STRING_SPECIAL 270
#define STATE_STRING_HEX1 271
#define STATE_STRING_HEX2 272

#define STATE_LINE_COMMENT 275
#define STATE_BLOCK_COMMENT_IN1 276
#define STATE_BLOCK_COMMENT_IN2 277
#define STATE_BLOCK_COMMENT 278
#define STATE_BLOCK_COMMENT_OUT1 279
#define STATE_BLOCK_COMMENT_OUT2 280

#define STATE_INT 283
#define STATE_FLOAT 284
#define STATE_FLOAT_E 285
#define STATE_FLOAT_EN 286
/////////////////////

///// GLOBALS /////
bool first_token = true;
Stack stack = {};
unsigned space_count = 0;
///////////////////

int scanner_init()
{
	return stack_init(&stack) && stack_push(&stack, 0);
}

void scanner_free()
{
	stack_free(&stack);
}

int get_next_token(Token *token)
{
	if(!stack.content || stack_empty(&stack))
		return ERROR_INTERNAL;
	unsigned state = STATE_INITIAL;
	String str;
	int c;
	bool break_while = false;
	while(!break_while && (c = fgetc(stdin)) != EOF)
	{
		switch(state)
		{
			// STATE_INTIAL decides which will be the next case
			case STATE_INITIAL:
				if(c == ' ' && !first_token)
				{
					continue;
				}
				else if(c == ' ' && first_token)
				{
					first_token = false;
					space_count++;
					state = STATE_INDENTATION;
				}
				else if(c == '\n' && !first_token)
				{
					token->type = TypeNewLine;
					first_token = true;
					break_while = true;
				}
				else if(c == '\n' && first_token)
				{
					continue;
				}
				else if(isalpha(c) && first_token)
				{
					first_token = false;
					ungetc(c, stdin);
					if(stack.content[stack.top] > 0)
					{
						stack_pop(&stack);
						if(stack.content[stack.top] != 0)
							first_token = true;
						token->type = TypeDedend;
						break_while = true;
					}
					else
					{
						continue;
					}
				}
				else if (c == '+')
				{
					token->type = TypeOperatorPlus;
					break_while = true;
				}
				else if(c == '-')
				{
					token->type = TypeOperatorMinus;
					break_while = true;
				}
				else if(c == '*')
				{
					token->type = TypeOperatorMul;
					break_while = true;
				}
				else if(c == '/')
				{
					token->type = TypeOperatorDiv;
					break_while = true;
				}
				else if(c == '=')
				{
					state = STATE_EQUALS;
				}
				else if(c == '(')
				{
					token->type = TypeLeftBracket;
					break_while = true;
				}
				else if(c == ')')
				{
					token->type = TypeRightBracket;
					break_while = true;
				}
				else if(c == ':')
				{
					token->type = TypeColon;
					break_while = true;
				}
				else if(c == ',')
				{
					token->type = TypeComma;
					break_while = true;
				}
				else if(c == '\t')
				{
					token->type = TypeTab;
					break_while = true;
				}
				else if(c == '>')
				{
					state = STATE_GREATER;
				}
				else if(c == '<')
				{
					state = STATE_LESSER;
				}
				else if(c == '!')
				{
					state = STATE_NEGATION;
				}
				else if(c == '#')
				{
					state = STATE_LINE_COMMENT;
				}
				else if(c == '\"')
				{
					state = STATE_BLOCK_COMMENT_IN1;
				}
				else if(c == '\'')
				{
					if(!str_init(&str))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(isdigit(c))
				{
					if(!str_init(&str))
						return ERROR_INTERNAL;
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_INT;
				}
				else if(isalpha(c) && !first_token)
				{
					if(!str_init(&str))
						return ERROR_INTERNAL;
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_VARIABLE;
				}
				else
				{
					DEBUG_PRINT("%s: 0x%.2X, %i, \'%c\'\n", "unknown symbol", c, c, c);
					return ERROR_LEXICAL;
				}
				break;
			case STATE_INDENTATION:
				if(c == ' ')
				{
					space_count++;
				}
				else if(c == '\n')
				{
					state = STATE_INITIAL;
					first_token = true;
				}
				else
				{
					ungetc(c, stdin);
					if(space_count > stack.content[stack.top])
					{
						if(!stack_push(&stack, space_count))
							return ERROR_INTERNAL;
						space_count = 0;
						token->type = TypeIndent;
						break_while = true;
					}
					else if(space_count < stack.content[stack.top])
					{
						stack_pop(&stack);
						if(space_count < stack.content[stack.top])
						{
							first_token = true;
							for(size_t i = 0; i < space_count; i++)
								ungetc(' ', stdin);
						}
						else if(space_count > stack.content[stack.top])
							return ERROR_LEXICAL;
						space_count = 0;
						token->type = TypeDedend;
						break_while = true;
					}
					else
					{
						space_count = 0;
						state = STATE_INITIAL;
					}
				}
				break;
			case STATE_EQUALS:
				if(c == '=')
					token->type = TypeEquality;
				else
				{
					ungetc(c, stdin);
					token->type = TypeAssignment;
				}
				break_while = true;
				break;
			case STATE_GREATER:
				if(c == '=')
					token->type = TypeGreaterEq;
				else
				{
					ungetc(c, stdin);
					token->type = TypeGreater;
				}
				break_while = true;
				break;
			case STATE_LESSER:
				if(c == '=')
					token->type = TypeLesserEq;
				else
				{
					ungetc(c, stdin);
					token->type = TypeLesser;
				}
				break_while = true;
				break;
			case STATE_NEGATION:
				if(c == '=')
					token->type = TypeUnEquality;
				else
				{
					ungetc(c, stdin);
					token->type = TypeNegation;
				}
				break_while = true;
				break;
			case STATE_LINE_COMMENT:
				if(c == '\n')
					state = STATE_INITIAL;
				break;
			case STATE_BLOCK_COMMENT_IN1:
				if(c == '\"')
					state = STATE_BLOCK_COMMENT_IN2;
				else
					return ERROR_LEXICAL;
				break;
			case STATE_BLOCK_COMMENT_IN2:
				if(c == '\"')
					state = STATE_BLOCK_COMMENT;
				else
					return ERROR_LEXICAL;
				break;
			case STATE_BLOCK_COMMENT:
				if(c == '\"')
					state = STATE_BLOCK_COMMENT_OUT1;
				break;
			case STATE_BLOCK_COMMENT_OUT1:
				if(c == '\"')
					state = STATE_BLOCK_COMMENT_OUT2;
				else
					state = STATE_BLOCK_COMMENT;
				break;
			case STATE_BLOCK_COMMENT_OUT2:
				if(c == '\"')
					state = STATE_INITIAL;
				else
					state = STATE_BLOCK_COMMENT;
				break;
			case STATE_STRING:
				if(c == '\'')
				{
					token->type = TypeString;
					if (!(token->data = malloc(sizeof(char) * str.asize + 1)))
					{
						str_free(&str);
						return ERROR_INTERNAL;
					}
					strcpy(token->data, str.content);
					str_free(&str);
					break_while = true;
				}
				else if(c == '\\')
				{
					state = STATE_STRING_SPECIAL;
				}
				else if(c == '\n')
				{
					return ERROR_LEXICAL;
				}
				else
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
				}
				break;
			// '\' in string
			case STATE_STRING_SPECIAL:
				if(c == 'n')
				{
					if(!str_add(&str, '\n'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == 't')
				{
					if(!str_add(&str, '\t'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == '\\')
				{
					if(!str_add(&str, '\\'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == '\'')
				{
					if(!str_add(&str, '\''))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == '\"')
				{
					if(!str_add(&str, '\"'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == 'x')
				{
					state = STATE_STRING_HEX1;
				}
				else
				{
					if(!str_add(&str, '\\'))
						return ERROR_INTERNAL;
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				break;
			case STATE_STRING_HEX1:
				if(isxdigit(c))
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_STRING_HEX2;
				}
				else
				{
					str_free(&str);
					return ERROR_LEXICAL;
				}
				break;
			case STATE_STRING_HEX2:
				if(isxdigit(c))
				{
					char cc[3] = { str_pop(&str), c, '\0'};
					char *end;
					long tmp = strtol(cc, &end, 16);
					if(*end != '\0')
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}
					if(!str_add(&str, tmp))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else
				{
					str_free(&str);
					return ERROR_LEXICAL;
				}
				break;
			// Adds char to dynamic string while c is alphanumeric, if it is not anymore
			// shoves the last char back into stdin and proceeds
			case STATE_VARIABLE:
				if(isalnum(c))
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
				}
				else
				{
					ungetc(c, stdin);

					token->type = TypeVariable;
					for(size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
					{
						if(strcmp(str.content, keywords[i]) == 0)
						{
							token->type = TypeKeyword;
							break;
						}
					}

					if (!(token->data = malloc(sizeof(char) * str.asize + 1)))
					{
						str_free(&str);
						return ERROR_INTERNAL;
					}
					strcpy(token->data, str.content);
					str_free(&str);
					break_while = true;
				}
				break;
			case STATE_INT:
				if(isdigit(c))
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
				}
				else if(c == '.')
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT;
				}
				else if(c == 'e' || c == 'E')
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT_E;
				}
				else
				{
					ungetc(c, stdin);
					token->type = TypeInt;

					if (!(token->data = malloc(sizeof(int))))
					{
						str_free(&str);
						return ERROR_INTERNAL;
					}
					char *end;
					long tmp = strtol(str.content, &end, 10);
					if(*end != '\0' || tmp > INT_MAX)
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}
					*(int*)token->data = tmp;
					str_free(&str);
					break_while = true;
				}
				break;
			case STATE_FLOAT:
				if(isdigit(c))
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
				}
				else if(c == 'e' || c == 'E')
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT_E;
				}
				else
				{
					ungetc(c, stdin);
					token->type = TypeFloat;

					if (!(token->data = malloc(sizeof(double))))
					{
						str_free(&str);
						return ERROR_INTERNAL;
					}
					char *end;
					double tmp = strtod(str.content, &end);
					if(*end != '\0')
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}
					*(double*)token->data = tmp;
					str_free(&str);
					break_while = true;
				}
				break;
			case STATE_FLOAT_E:
				if(isdigit(c) || c == '+' || c == '-')
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT_EN;
				}
				else
				{
					str_free(&str);
					return ERROR_LEXICAL;
				}
				break;
			case STATE_FLOAT_EN:
				if(isdigit(c))
				{
					if(!str_add(&str, c))
						return ERROR_INTERNAL;
				}
				else
				{
					ungetc(c, stdin);
					token->type = TypeFloat;

					if (!(token->data = malloc(sizeof(double))))
					{
						str_free(&str);
						return ERROR_INTERNAL;
					}
					char *end;
					double tmp = strtod(str.content, &end);
					if(*end != '\0')
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}
					*(double*)token->data = tmp;
					str_free(&str);
					break_while = true;
				}
				break;
		}
	}
	//eof in state, where it shouldnt be. isnt finished -> todo
	if(c == EOF && !break_while && state != STATE_INITIAL)
		return ERROR_LEXICAL;
	else if (c == EOF)
		return EOF;
	else
		return 0;
}

#if defined(DEBUG) && DEBUG > 0
const char *type_names[] = {"+", "-", "*", "/", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "tab", "new line", "keyword", "variable", "string", "int", "float", "indent", "dedent"};
int scanner_main()
{
	Token token = {};
	int err_num = 0;

	scanner_init();

	while((err_num = get_next_token(&token)) == 0)
	{
		if(token.type == TypeString || token.type == TypeVariable || token.type == TypeKeyword)
			printf("Token type: %s | Token data: %s \n", type_names[token.type], (char*)token.data);
		else if(token.type == TypeInt)
			printf("Token type: %s | Token data: %i \n", type_names[token.type], *(int*)token.data);
		else if(token.type == TypeFloat)
			printf("Token type: %s | Token data: %f \n", type_names[token.type], *(double*)token.data);
		else
			printf("Token type: %s \n", type_names[token.type]);
		free(token.data);
		token.data = NULL;
	}
	free(token.data);

	scanner_free();

	switch (err_num)
	{
		case ERROR_LEXICAL:
			DEBUG_PRINT("%s\n", "ERROR_LEXICAL");
			break;
		case ERROR_INTERNAL:
			DEBUG_PRINT("%s\n", "ERROR_INTERNAL");
			break;
		case EOF:
			printf("EOF\n");
			err_num = 0;
			break;
	}
	return err_num;
}
#endif
