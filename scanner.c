#include "scanner.h"

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


/// JUST FOR TESTING //
const char *opNames[] = {"+", "-", "*", "/", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "tab", "new line", "keyword", "variable", "string", "int", "float", "indent", "dedent"};
//////////////////////


///// GLOBALS /////
FILE *_stream;
bool first_token = true;
Stack stack = {};
unsigned space_count = 0;
///////////////////

int scanner_init()
{
	_stream = stdin;
	return stack_init(&stack) && stack_push(&stack, 0);
}

void scanner_free()
{
	stack_free(&stack);
}

void scanner_set_stream(FILE *stream)
{
	_stream = stream;
}

int get_next_token(Token *token)
{
	if(!stack.content || stack_empty(&stack))
		return ERROR_INTERNAL;
	unsigned state = STATE_INITIAL;
	String str;
	int c;
	bool break_while = false;
	while(!break_while && (c = fgetc(_stream)) != EOF)
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
				else if((isalpha(c) || c == '_') && first_token)
				{
					first_token = false;
					ungetc(c, _stream);
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
				/*else if(c == '\t')
				{
					token->type = TypeTab;
					break_while = true;
				}*/
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
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_INT;
				}
				else if((isalpha(c) || c == '_') && !first_token)
				{
					if(!str_init(&str))
						return ERROR_INTERNAL;
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_VARIABLE;
				}
				else
				{
					fprintf(stderr, "%s: 0x%.2X, %i\n", "unknown symbol", c, c);
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
					ungetc(c, _stream);
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
								ungetc(' ', _stream);
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
					ungetc(c, _stream);
					token->type = TypeAssignment;
				}
				break_while = true;
				break;
			case STATE_GREATER:
				if(c == '=')
					token->type = TypeGreaterEq;
				else
				{
					ungetc(c, _stream);
					token->type = TypeGreater;
				}
				break_while = true;
				break;
			case STATE_LESSER:
				if(c == '=')
					token->type = TypeLesserEq;
				else if(c == '>')
					token->type = TypeUnEquality;
				else
				{
					ungetc(c, _stream);
					token->type = TypeLesser;
				}
				break_while = true;
				break;
			case STATE_NEGATION:
				if(c == '=')
					token->type = TypeUnEquality;
				else
				{
					ungetc(c, _stream);
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
				else if(c > 31)
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
				}
				else
				{
					return ERROR_LEXICAL;
				}
				break;
			// '\' in string
			case STATE_STRING_SPECIAL:
				if(c == 'n')
				{
					if(!str_pushc(&str, '\n'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == 't')
				{
					if(!str_pushc(&str, '\t'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == '\\')
				{
					if(!str_pushc(&str, '\\'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == '\'')
				{
					if(!str_pushc(&str, '\''))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == '\"')
				{
					if(!str_pushc(&str, '\"'))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(c == 'x')
				{
					state = STATE_STRING_HEX1;
				}
				else
				{
					if(!str_pushc(&str, '\\'))
						return ERROR_INTERNAL;
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				break;
			case STATE_STRING_HEX1:
				if(isxdigit(c))
				{
					if(!str_pushc(&str, c))
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
					if(!str_pushc(&str, tmp))
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
			// shoves the last char back into _stream and proceeds
			case STATE_VARIABLE:
				if(isalnum(c) || c == '_')
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
				}
				else
				{
					ungetc(c, _stream);

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
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
				}
				else if(c == '.')
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT;
				}
				else if(c == 'e' || c == 'E')
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT_E;
				}
				else
				{
					ungetc(c, _stream);
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
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
				}
				else if(c == 'e' || c == 'E')
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT_E;
				}
				else
				{
					ungetc(c, _stream);
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
					if(!str_pushc(&str, c))
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
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
				}
				else
				{
					ungetc(c, _stream);
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
const char *type_names[] = {"+", "-", "*", "/", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", /*"tab", */"new line", "keyword", "variable", "string", "int", "float", "indent", "dedent"};
int scanner_main()
{
	Token token = {};
	int err_num = 0;

	scanner_init();

	while((err_num = get_next_token(&token)) == 0)
	{
		if(token.type == TypeString || token.type == TypeVariable || token.type == TypeKeyword)
			printf("AToken type: %s | Token data: %s \n", opNames[token.type], (char*)token.data);
		else if(token.type == TypeInt)
			printf("Token type: %s | Token data: %i \n", opNames[token.type], *(int*)token.data);
		else if(token.type == TypeFloat)
			printf("Token type: %s | Token data: %f \n", opNames[token.type], *(double*)token.data);
		else
			printf("Token type: %s \n", opNames[token.type]);
		free(token.data);
		token.data = NULL;
	}
	free(token.data);

	scanner_free();

	switch (err_num)
	{
		case ERROR_LEXICAL:
			fprintf(stderr, "%s\n", "ERROR_LEXICAL");
			break;
		case ERROR_INTERNAL:
			fprintf(stderr, "%s\n", "ERROR_INTERNAL");
			break;
		case EOF:
			printf("EOF\n");
			err_num = 0;
			break;
	}
	return err_num;
}
