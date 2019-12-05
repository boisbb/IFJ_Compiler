#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

typedef enum
{
	TypeOperatorPlus,
	TypeOperatorMinus,
	TypeOperatorMul,
	TypeOperatorDiv,
	TypeOperatorFloorDiv,
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
	TypeVariable, // 19
	TypeString, // 20
	TypeDocString, // 21
	TypeInt, // 22
	TypeFloat,
	TypeIndent,
	TypeDedend,

	TypeKeywordNone,
	TypeKeywordPass,

	// Following tokens cant be returned from scanner
	TypeUnspecified, // For adding variables into symtable
	TypeFunc, // For adding functions into symtable
	TypeUndefined // If the function is not returning any value
} Type;
/////////////////////

// TOKEN STRUCTURE ///
typedef struct
{
	Type type;
	void *data;
} Token;
////////////////////

// function, that should be called before using function scanner
// returns 0 on error and 1 on success
int scanner_init();

// function, that should be called at the end of using scanner
void scanner_free();

// set input stream, default stdin, can be called after scanner_init!
void scanner_set_stream(FILE *stream);

// reads one token from stdin a write it to parameter
// returns 0 on success, EOF, ERROR_LEXICAL, ERROR_INTERNAL
// call scanner_init before and scanner_free after using!!
int get_next_token(Token *token);

#endif
