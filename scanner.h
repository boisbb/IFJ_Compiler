#ifndef SCANNER_H
#define SCANNER_H

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
	TypeTab,
	TypeNewLine,
	TypeKeyword,
	TypeVariable,
	TypeString,
	TypeInt,
	TypeFloat,
	TypeIndent,
	TypeDedend
} Type;

typedef struct
{
	Type type;
	void *data;
} Token;

/// JUST FOR TESTING ///
#if defined(DEBUG) && DEBUG > 0
 int scanner_main();
#endif
///////////////////////

// function, that should be called before using function scanner
// returns 0 on error and 1 on success
int scanner_init();

// function, that should be called at the end of using scanner
void scanner_free();

// reads one token from stdin a write it to parameter
// returns 0 on success, EOF, ERROR_LEXICAL, ERROR_INTERNAL
// call scanner_init before and scanner_free after using!!
int get_next_token(Token *token);

#endif
