#ifndef GENERATOR_FUNCTIONS_H
#define GENERATOR_FUNCTIONS_H

#include "error.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

const char fnc_inputs[] =
"\n# Predefined function inputs BEGIN\n"
"LABEL $inputs\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"READ LF@%retval string\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function inputs END\n";

const char fnc_inputi[] =
"\n# Predefined function inputi BEGIN\n"
"LABEL $inputi\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"READ LF@%retval int\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function inputi END\n";

const char fnc_inputf[] =
"\n# Predefined function inputf BEGIN\n"
"LABEL $inputf\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"READ LF@%retval float\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function inputf END\n";

const char fnc_print[] =
"\n# Predefined function print BEGIN\n"
"LABEL $print\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"MOVE LF@%retval string@None\n"
"DEFVAR LF@counter\n"
"MOVE LF@counter int@0\n"
"DEFVAR LF@tmp\n"
"LABEL $print%while\n"
"JUMPIFEQ $print%end LF@counter LF@%0\n"
"POPS LF@tmp\n"
"WRITE LF@tmp\n"
"JUMPIFEQ $print%end LF@counter LF@%0\n"
"WRITE string@\\032\n"
"ADD LF@counter LF@counter int@1\n"
"JUMP $print%while\n"
"LABEL $print%end\n"
"WRITE string@\\010\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function print END\n";

const char fnc_len[] =
"\n# Predefined function len BEGIN\n"
"LABEL $len\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"STRLEN LF@%retval LF@%0\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function len END\n";

const char fnc_substr[] =
"\n# Predefined function substr BEGIN\n"
"LABEL $substr\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"MOVE LF@%retval string@None\n"
"DEFVAR LF@len\n"
"STRLEN LF@len LF@%0\n"
"DEFVAR LF@exp_res\n"
"LT LF@exp_res LF@%1 int@0\n"
"JUMPIFEQ $substr%return LF@exp_res bool@true\n"
"GT LF@exp_res LF@%1 LF@len\n"
"JUMPIFEQ $substr%return LF@exp_res bool@true\n"
"LT LF@exp_res LF@%2 int@0\n"
"JUMPIFEQ $substr%return LF@exp_res bool@true\n"
"DEFVAR LF@counter\n"
"SUB LF@len LF@len LF@%1\n"
"SUB LF@len LF@len int@1\n"
"GT LF@exp_res LF@%2 LF@len\n"
"JUMPIFEQ $substr%counter LF@exp_res bool@false\n"
"MOVE LF@counter LF@len\n"
"JUMP $substr%counter_end\n"
"LABEL $substr%counter\n"
"MOVE LF@counter LF@%2\n"
"LABEL $substr%counter_end\n"
"MOVE LF@%retval string@\n"
"DEFVAR LF@char\n"
"LABEL $substr%while\n"
"JUMPIFEQ $substr%return LF@counter int@0\n"
"GETCHAR LF@char LF@%0 LF@%1\n"
"CONCAT LF@%retval LF@%retval LF@char\n"
"SUB LF@counter LF@counter int@1\n"
"ADD LF@%1 LF@%1 int@1\n"
"JUMP $substr%while\n"
"LABEL $substr%return\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function substr END\n";

const char fnc_ord[] =
"\n# Predefined function ord BEGIN\n"
"LABEL $ord\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"MOVE LF@%retval string@None\n"
"DEFVAR LF@exp_res\n"
"LT LF@exp_res LF@%1 int@0\n"
"JUMPIFEQ $ord%return LF@exp_res bool@true\n"
"STRLEN LF@exp_res LF@%0\n"
"SUB LF@exp_res LF@exp_res int@1\n"
"GT LF@exp_res LF@%1 LF@exp_res\n"
"JUMPIFEQ $ord%return LF@exp_res bool@true\n"
"STRI2INT LF@%retval LF@%0 LF@%1\n"
"LABEL $ord%return\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function ord END\n";

const char fnc_chr[] =
"\n# Predefined function chr BEGIN\n"
"LABEL $chr\n"
"PUSHFRAME\n"
"DEFVAR LF@%retval\n"
"MOVE LF@%retval string@None\n"
"DEFVAR LF@exp_res\n"
"LT LF@exp_res LF@%0 int@0\n"
"JUMPIFEQ $chr%error LF@exp_res bool@true\n"
"GT LF@exp_res LF@%0 int@255\n"
"JUMPIFEQ $chr%error LF@exp_res bool@true\n"
"INT2CHAR LF@%retval LF@%0\n"
"JUMP $chr%return\n"
"LABEL $chr%error\n"
"EXIT int@" STR(ERROR_SEMANTIC_RUNTIME) "\n"
"LABEL $chr%return\n"
"POPFRAME\n"
"RETURN\n"
"# Predefined function chr END\n";

#endif
