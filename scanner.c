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
const char *keywords[] = { "def", "else", "if", "None", "pass", "return", "while" };
const Type keyword_types[] = { TypeUndefined, TypeUndefined, TypeUndefined, TypeKeywordNone, TypeKeywordPass, TypeUndefined, TypeUndefined };
const char *type_names[] = {"+", "-", "*", "/", "//", "=", "==", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", /*"tab", */"new line", "keyword", "variable", "string", "documentary string", "int", "float", "indent", "dedent", "None", "pass"};
/////// STATES ///////
#define STATE_INITIAL 259

#define STATE_EQUALS 260
#define STATE_GREATER 261
#define STATE_LESSER 262
#define STATE_NEGATION 263

#define STATE_DIV 264

#define STATE_INDENTATION 267

#define STATE_VARIABLE 268

#define STATE_STRING 269
#define STATE_STRING_SPECIAL 270
#define STATE_STRING_HEX1 271
#define STATE_STRING_HEX2 272

#define STATE_LINE_COMMENT 275

#define STATE_DOC_STRING_IN1 276
#define STATE_DOC_STRING_IN2 277
#define STATE_DOC_STRING 278
#define STATE_DOC_STRING_OUT1 279
#define STATE_DOC_STRING_OUT2 280
#define STATE_DOC_STRING_SPECIAL 281

#define STATE_INT 283
#define STATE_FLOAT 284
#define STATE_FLOAT_E 285
#define STATE_FLOAT_EN 286
/////////////////////

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

int get_next_token_internal(Token *token)
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
					first_token = false;
					token->type = TypeOperatorPlus;
					break_while = true;
				}
				else if(c == '-')
				{
					first_token = false;
					token->type = TypeOperatorMinus;
					break_while = true;
				}
				else if(c == '*')
				{
					first_token = false;
					token->type = TypeOperatorMul;
					break_while = true;
				}
				else if(c == '/')
				{
					first_token = false;
					state = STATE_DIV;
				}
				else if(c == '=')
				{
					first_token = false;
					state = STATE_EQUALS;
				}
				else if(c == '(')
				{
					first_token = false;
					token->type = TypeLeftBracket;
					break_while = true;
				}
				else if(c == ')')
				{
					first_token = false;
					token->type = TypeRightBracket;
					break_while = true;
				}
				else if(c == ':')
				{
					first_token = false;
					token->type = TypeColon;
					break_while = true;
				}
				else if(c == ',')
				{
					first_token = false;
					token->type = TypeComma;
					break_while = true;
				}
				/*else if(c == '\t')
				{
					first_token = false;
					token->type = TypeTab;
					break_while = true;
				}*/
				else if(c == '>')
				{
					first_token = false;
					state = STATE_GREATER;
				}
				else if(c == '<')
				{
					first_token = false;
					state = STATE_LESSER;
				}
				else if(c == '!')
				{
					first_token = false;
					state = STATE_NEGATION;
				}
				else if(c == '#')
				{
					state = STATE_LINE_COMMENT;
				}
				else if(c == '\"')
				{
					first_token = false;
					state = STATE_DOC_STRING_IN1;
				}
				else if(c == '\'')
				{
					first_token = false;
					if(!str_init(&str))
						return ERROR_INTERNAL;
					state = STATE_STRING;
				}
				else if(isdigit(c))
				{
					first_token = false;
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
					DEBUG_PRINT("%s: 0x%.2X, %i\n", "unknown symbol", c, c);
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
			case STATE_DIV:
				if(c == '/')
				{
					token->type = TypeOperatorFloorDiv;
				}
				else
				{
					ungetc(c, _stream);
					token->type = TypeOperatorDiv;
				}
				break_while = true;
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
				{
					ungetc(c, _stream);
					state = STATE_INITIAL;
				}
				break;
			case STATE_DOC_STRING_IN1:
				if(c == '\"')
					state = STATE_DOC_STRING_IN2;
				else
					return ERROR_LEXICAL;
				break;
			case STATE_DOC_STRING_IN2:
				if(c == '\"')
				{
					if(!str_init(&str))
						return ERROR_INTERNAL;
					state = STATE_DOC_STRING;
				}
				else
					return ERROR_LEXICAL;
				break;
			case STATE_DOC_STRING:
				if(c == '\"')
					state = STATE_DOC_STRING_OUT1;
				else if(c == '\\')
					state = STATE_DOC_STRING_SPECIAL;
				else
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
				}
				break;
			case STATE_DOC_STRING_SPECIAL:
				if(c == '\"')
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_DOC_STRING;
				}
				else
				{
					if(!str_pushc(&str, '\\'))
						return ERROR_INTERNAL;
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_DOC_STRING;
				}
				break;
			case STATE_DOC_STRING_OUT1:
				if(c == '\"')
					state = STATE_DOC_STRING_OUT2;
				else
					state = STATE_DOC_STRING;
				break;
			case STATE_DOC_STRING_OUT2:
				if(c == '\"')
				{
					token->type = TypeDocString;
					if (!(token->data = malloc(sizeof(char) * str.asize + 1)))
					{
						str_free(&str);
						return ERROR_INTERNAL;
					}
					strcpy(token->data, str.content);
					str_free(&str);
					break_while = true;
				}
				else
					state = STATE_DOC_STRING;
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

					bool cpy = true;
					token->type = TypeVariable;
					for(size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
					{
						if(strcmp(str.content, keywords[i]) == 0)
						{
							if(keyword_types[i] != TypeUndefined)
							{
								token->type = keyword_types[i];
								cpy = false;
							}
							else
							{
								token->type = TypeKeyword;
							}
							break;
						}
					}

					if(cpy)
					{
						if (!(token->data = malloc(sizeof(char) * str.asize + 1)))
						{
							str_free(&str);
							return ERROR_INTERNAL;
						}
						strcpy(token->data, str.content);
					}
					str_free(&str);
					break_while = true;
				}
				break;
			case STATE_INT:
				if(isdigit(c))
				{
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					if(str.content[0] == '0' && str.content[1] == '0')
						return ERROR_LEXICAL;
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
				else if(isalpha(c))
					return ERROR_LEXICAL;
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
					if(str.content[str.asize-1] == '.')
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}
					
					if(!str_pushc(&str, c))
						return ERROR_INTERNAL;
					state = STATE_FLOAT_E;
				}
				else if(isalpha(c))
					return ERROR_LEXICAL;
				else
				{
					ungetc(c, _stream);
					token->type = TypeFloat;

					if(str.content[str.asize-1] == '.')
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}

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
				else if(isalpha(c))
					return ERROR_LEXICAL;
				else
				{
					ungetc(c, _stream);
					token->type = TypeFloat;

					if(str.content[str.asize-1] == 'e' || str.content[str.asize-1] == 'E' || str.content[str.asize-1] == '+' || str.content[str.asize-1] == '-')
					{
						str_free(&str);
						return ERROR_LEXICAL;
					}

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

#if defined(DEBUG) && DEBUG > 0
	if(c != EOF)
	{
		if(token->type == TypeString || token->type == TypeVariable || token->type == TypeKeyword || token->type == TypeDocString)
			DEBUG_PRINT("Token type: %s | Token data: %s \n", type_names[token->type], (char*)token->data);
		else if(token->type == TypeInt)
			DEBUG_PRINT("Token type: %s | Token data: %i \n", type_names[token->type], *(int*)token->data);
		else if(token->type == TypeFloat)
			DEBUG_PRINT("Token type: %s | Token data: %f \n", type_names[token->type], *(double*)token->data);
		else
			DEBUG_PRINT("Token type: %s \n", type_names[token->type]);
	}
#endif

	//eof in state, where it shouldnt be. isnt finished -> todo
	if(c == EOF && !break_while && state != STATE_INITIAL)
		return ERROR_LEXICAL;
	else if (c == EOF)
		return EOF;
	else
		return NO_ERROR;
}

int get_next_token(Token *token)
{
	int ret = get_next_token_internal(token);
	if(ret > 0)
		exit(ret);
	return ret;
}
