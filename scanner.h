#include <stdio.h>
#include <stdlib.h>//malloc, strtol, strtod
#include <stdbool.h>
#include <ctype.h>//isalpha, isalnum, isdigit, isxdigit
#include <string.h>//strcpy, strcmp
#include <limits.h>//INT_MAX

#include "error.h"
#include "strings.h"
#include "stack.h"


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

//// ENUM OF TYPES ///
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
/////////////////////



// TOKEN STRUCTURE ///
typedef struct
{
	Type type;
	void *data;
} Token;
////////////////////



int scanner_init();

int get_next_token(Token *token);

int scanner_main();
