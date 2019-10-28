#include <stdio.h>
#include <stdlib.h>//malloc
#include <stdbool.h>
#include <ctype.h>//isalpha, isalnum
#include <string.h>//strcpy, strcmp

#include "strings.h"
#include "error.h"

/////// CONSTANTS ////
const char *keywords[] = {"def", "else", "if", "None", "pass", "return", "while"};

/////// STATES ///////
#define STATE_INITIAL 259

#define STATE_EQUALS 260
#define STATE_GREATER 261
#define STATE_LESSER 262
#define STATE_NEGATION 263

#define STATE_VARIABLE 268

#define STATE_STRING 269
#define STATE_STRING_SPECIAL 270

#define STATE_LINE_COMMENT 271
#define STATE_BLOCK_COMMENT_IN1 272
#define STATE_BLOCK_COMMENT_IN2 273
#define STATE_BLOCK_COMMENT 274
#define STATE_BLOCK_COMMENT_OUT1 275
#define STATE_BLOCK_COMMENT_OUT2 276
/////////////////////

//// ENUM OF TYPES ///
typedef enum
{
    TypeOperatorPlus,
    TypeOperatorMinus,
    TypeOperatorMul,
    TypeOperatorDiv,
    TypeEquality,
    TypeGreater,
    TypeGreaterEq,
    TypeLesser,
    TypeLesserEq,
    TypeNegation,
    TypeUnEquality,
    TypeLeftBracket,
    TypeRightBracket,
    TypeColon,
    TypeComma,
    TypeTab,
    TypeNewLine,
    TypeKeyword,
    TypeVariable,
    TypeString
} Type;
/////////////////////

/// JUST FOR TESTING //
const char *opNames[] = {"+", "-", "*", "/", "=", ">", ">=", "<", "<=", "!", "!=", "(", ")", ":", ",", "tab", "new line", "keyword", "variable", "string"};
//////////////////////

// TOKEN STRUCTURE ///
typedef struct
{
    Type type;
    char *string;
} Token;
////////////////////


// In actuality, there will be another condition, that the cycle will continue only if parser asks for token,
// the function will cycle, but won't do anything
// In the future, this will be function named get_next_token
int get_next_token(Token *token)
{
    size_t state = STATE_INITIAL;
    String *str = malloc(sizeof(String));
    if(!str)
        return ERROR_INTERNAL;

    int c;
    bool break_while = false;
    while(!break_while && (c = fgetc(stdin)) != EOF)
    {
        switch(state)
        {
            // STATE_INTIAL decides which will be the next case
            case STATE_INITIAL:
                if(c == ' ')
                {
                    continue;
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
                    token->type = TypeEquality;
                    break_while = true;
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
                else if(c == '\n')
                {
                    token->type = TypeNewLine;
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
                    if(!str_init(str))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                else if(isalpha(c))
                {
                    if(!str_init(str))
                        return ERROR_INTERNAL;
                    if(!str_add(str, c))
                        return ERROR_INTERNAL;
                    state = STATE_VARIABLE;
                }
                else
                {
                    fprintf(stderr, "%s: 0x%.2X, %i\n", "unknown symbol", c, c);
                    return ERROR_LEXICAL;
                }
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
                    if (!(token->string = malloc(sizeof(char) * str->asize)))
                        return ERROR_INTERNAL;
                    strcpy(token->string, str->contents);
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
                    if(!str_add(str, c))
                        return ERROR_INTERNAL;
                }
                break;
            // '\' in string
            case STATE_STRING_SPECIAL:
                if(c == 'n')
                {
                    if(!str_add(str, '\n'))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                else if(c == 't')
                {
                    if(!str_add(str, '\t'))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                else if(c == '\\')
                {
                    if(!str_add(str, '\\'))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                else if(c == '\'')
                {
                    if(!str_add(str, '\''))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                else if(c == '\"')
                {
                    if(!str_add(str, '\"'))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                else if(c == 'x')
                {
                    //todo
                    state = STATE_STRING;
                }
                else
                {
                    if(!str_add(str, '\\'))
                        return ERROR_INTERNAL;
                    if(!str_add(str, c))
                        return ERROR_INTERNAL;
                    state = STATE_STRING;
                }
                break;
            // Adds char to dynamic string while c is alphanumeric, if it is not anymore
            // shoves the last char back into stdin and proceeds
            case STATE_VARIABLE:
                if(isalnum(c))
                {
                    if(!str_add(str, c))
                        return ERROR_INTERNAL;
                }
                else
                {
                    ungetc(c, stdin);

                    token->type = TypeVariable;
                    for(size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
                    {
                        if(strcmp(str->contents, keywords[i]) == 0)
                        {
                            token->type = TypeKeyword;
                            break;
                        }
                    }

                    if (!(token->string = malloc(sizeof(char) * str->asize)))
                        return ERROR_INTERNAL;
                    strcpy(token->string, str->contents);
                    break_while = true;
                }
                break;
        }
    }
    //eof in state, where it shouldnt be
    if(c == EOF && !break_while && state != STATE_INITIAL)
        return ERROR_LEXICAL;
    else if (c == EOF)
        return EOF;
    else
        return 0;
}

int main()
{
    Token token;
    int err_num = 0;

    while((err_num = get_next_token(&token)) == 0)
    {
        printf("Token type: %s | Token string: %s \n", opNames[token.type], token.string);
        free(token.string);
        token.string = NULL;
    }

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
