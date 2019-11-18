#include <stdio.h>
#include <stdlib.h>//malloc, strtol, strtod
#include <stdbool.h>
#include <ctype.h>//isalpha, isalnum, isdigit, isxdigit
#include <string.h>//strcpy, strcmp
#include <limits.h>//INT_MAX

#include <stdio.h>//fgetc, _stream, EOF, ungetc

typedef enum
{
	TypeOperatorPlus,
	TypeOperatorMinus,
	TypeOperatorMul,
	TypeOperatorDiv,
	TypeAssignment,
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
	//TypeTab, //useless
	TypeNewLine,
	TypeKeyword,
	TypeVariable,
	TypeString,
	TypeInt,
	TypeFloat,
	TypeIndent,
	TypeDedend,
	TypeUnspecified, // For adding variables into symtable
	TypeFunc // For adding functions into symtable
} Type;
/////////////////////



// TOKEN STRUCTURE ///
typedef struct
{
	Type type;
	void *data;
} Token;
////////////////////



int scanner_init();

//set input stream, default stdin, can be called after scanner_init!
void scanner_set_stream(FILE *stream);

// reads one token from stdin a write it to parameter
// returns 0 on success, EOF, ERROR_LEXICAL, ERROR_INTERNAL
// call scanner_init before and scanner_free after using!!
int get_next_token(Token *token);

int scanner_main();
