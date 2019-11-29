#include "generator.h"
#include "generator_functions.h"
#include "strings.h"

//GLOBALS
String code = {};
String fnc = {};
bool inside_fnc = true;

size_t if_label_counter = 0;
size_t while_label_counter = 0;
size_t control_label_counter = 0;
/////////

#define ADD_CODE(str) \
	(inside_fnc ? str_push(&fnc, (str)) : str_push(&code, (str)))

#define ADD_LINE(line) \
	ADD_CODE(line "\n")

static inline bool get_str_from_index(char* str, unsigned index)
{
	return sprintf(str, "%d", index) > 0;
}

static inline bool generate_header()
{
	return
		ADD_LINE("# Program BEGIN") &&
		ADD_LINE(".IFJcode19") &&
		ADD_LINE("DEFVAR GF@%fnc_return") &&
		ADD_LINE("DEFVAR GF@%exp_result") &&
		ADD_LINE("DEFVAR GF@%exp_result%type") &&
		ADD_LINE("DEFVAR GF@%stmp1") &&
		ADD_LINE("DEFVAR GF@%stmp2") &&
		ADD_LINE("DEFVAR GF@%stmp1%type") &&
		ADD_LINE("DEFVAR GF@%stmp2%type") &&
		ADD_LINE("JUMP $$main");
}

static inline bool generate_predefined_functions()
{
	return
		ADD_CODE(fnc_inputs) &&
		ADD_CODE(fnc_inputi) &&
		ADD_CODE(fnc_inputf) &&
		ADD_CODE(fnc_print) &&
		ADD_CODE(fnc_len) &&
		ADD_CODE(fnc_substr) &&
		ADD_CODE(fnc_ord) &&
		ADD_CODE(fnc_chr);
}

static inline bool generate_var_value(Type type, void* data)
{
	String tmp;
	bool ret = str_init(&tmp) && str_set_size(&tmp, MAX_DIGITS_DOUBLE);
	if(ret)
	{
		switch(type)
		{
			case TypeInt:
				if(data)
				{
					ret = sprintf(tmp.content, "%d", *(int*)data) > 0 &&
						ADD_CODE("int@") && ADD_CODE(tmp.content);
				}
				else
				{
					ret = ADD_CODE("int@0");
				}
				break;
			case TypeFloat:
				if(data)
				{
					ret = sprintf(tmp.content, "%a", *(double*)data) > 0 &&
						ADD_CODE("float@") && ADD_CODE(tmp.content);
				}
				else
				{
					ret = ADD_CODE("float@0x0p+0");
				}
				break;
			case TypeString:
				if(data)
				{
					for(size_t i = 0; i < strlen((char*)data); i++)
					{
						if(((char*)data)[i] < 33 || ((char*)data)[i] == 35 || ((char*)data)[i] == 92)
						{
							char str_tmp[4];
							ret &= sprintf(str_tmp, "%03d", ((char*)data)[i]) > 0 &&
								str_pushc(&tmp, '\\') && str_push(&tmp, str_tmp);
						}
						else
							ret &= str_pushc(&tmp, ((char*)data)[i]);
					}
					ret &= ADD_CODE("string@") && ADD_CODE(tmp.content);
				}
				else
				{
					ret = ADD_CODE("string@");
				}
				break;
			case TypeVariable:
				if(data)
					ret = ADD_CODE("LF@") && ADD_CODE(data);
				else
					ret = false;
				break;
			default:
				ret = false;
				break;
		}
	}
	str_free(&tmp);
	return ret;
}

bool generate_unique_label(char* dest, LabelType type)
{
	switch (type)
	{
		case LABEL_IF:
			return sprintf(dest, "if%ld", if_label_counter++) > 0;
			break;
		case LABEL_WHILE:
			return sprintf(dest, "while%ld", while_label_counter++) > 0;
			break;
		case LABEL_CONTROL:
			return sprintf(dest, "control%ld", control_label_counter++) > 0;
			break;
	}
	return false;
}

bool generator_init()
{
	return
		str_init(&code) &&
		str_init(&fnc) &&
		generate_header() &&
		generate_predefined_functions();
}

void generator_free()
{
	str_free(&fnc);
	str_free(&code);
}

char *generator_code_get()
{
	str_push(&fnc, code.content);
	return fnc.content;
}

void generator_code_clear()
{
	str_clear(&code);
}

bool generate_main_begin()
{
	inside_fnc = false;
	return
		ADD_LINE("\n# Main scope BEGIN") &&
		ADD_LINE("LABEL $$main") &&
		ADD_LINE("CREATEFRAME") &&
		ADD_LINE("PUSHFRAME");
}
bool generate_main_end()
{
	inside_fnc = false;
	return
		ADD_LINE("POPFRAME") &&
		ADD_LINE("CLEARS") &&
		ADD_LINE("# Main scope END");
}

bool generate_var_declaration(char* label, bool scope)
{
	return
		(scope ? ADD_CODE("DEFVAR GF@") : ADD_CODE("DEFVAR LF@")) &&
		ADD_CODE(label) &&
		ADD_CODE("\n");
}

bool generate_var_definition(char* label, bool scope, Type type, void* data)
{
	return
		(scope ? ADD_CODE("MOVE GF@") : ADD_CODE("MOVE LF@")) &&
		ADD_CODE(label) &&
		ADD_CODE(" ") &&
		generate_var_value(type, data) &&
		ADD_CODE("\n");
}

bool generate_push_data(Type type, void* data)
{
	return
		ADD_CODE("PUSHS ") && generate_var_value(type, data) && ADD_CODE("\n");
}
bool generate_push_var(char* label, bool scope)
{
	return
		(scope ? ADD_CODE("PUSHS GF@") : ADD_CODE("PUSHS LF@")) && ADD_CODE(label) && ADD_CODE("\n");
}
bool generate_push_var_unspecified(char* label, char* var_label, bool scope, Type type)
{
	bool ret;
	switch(type)
	{
		case TypeString:
			ret =
			ADD_CODE("TYPE GF@%stmp1%type ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("string GF@%stmp1%type string@string\n") &&
			ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("string\n") &&
			ADD_CODE("PUSHS ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n");
			break;
		case TypeInt:
			ret =
			ADD_CODE("TYPE GF@%stmp1%type ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("int GF@%stmp1%type string@int\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("float GF@%stmp1%type string@float\n") &&
			ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("float\n") &&
			ADD_CODE("PUSHS ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_LINE("FLOAT2INTS") &&
			ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("end\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("int\n") &&
			ADD_CODE("PUSHS ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("end\n");
			break;
		case TypeFloat:
			ret =
			ADD_CODE("TYPE GF@%stmp1%type ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("int GF@%stmp1%type string@int\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("float GF@%stmp1%type string@float\n") &&
			ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("int\n") &&
			ADD_CODE("PUSHS ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_LINE("INT2FLOATS") &&
			ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("end\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("float\n") &&
			ADD_CODE("PUSHS ") && (scope ? ADD_CODE("GF@") : ADD_CODE("LF@")) && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE("end\n");
			break;
		default:
			return false;
			break;
	}
	return ret;
}

bool generate_operation(Type type)
{
	bool ret;
	switch(type)
	{
		case TypeOperatorPlus:
			ret =
			ADD_LINE("ADDS");
			break;
		case TypeOperatorMinus:
			ret =
			ADD_LINE("SUBS");
			break;
		case TypeOperatorMul:
			ret =
			ADD_LINE("MULS");
			break;
		case TypeOperatorDiv:
			ret =
			ADD_LINE("DIVS");
			break;
		case TypeOperatorFloorDiv:
			ret =
			ADD_LINE("POPS GF@%stmp1") &&
			ADD_LINE("INT2FLOATS") &&
			ADD_LINE("PUSHS GF@%stmp1") &&
			ADD_LINE("INT2FLOATS") &&
			ADD_LINE("DIVS") &&
			ADD_LINE("FLOAT2INTS");
			break;
		case TypeEquality:
			ret =
			ADD_LINE("EQS");
			break;
		case TypeUnEquality:
			ret =
			ADD_LINE("EQS") &&
			ADD_LINE("NOTS");
			break;
		case TypeLesser:
			ret =
			ADD_LINE("LTS");
			break;
		case TypeGreater:
			ret =
			ADD_LINE("GTS");
			break;
		case TypeLesserEq:
			ret =
			ADD_LINE("POPS GF@%stmp1") &&
			ADD_LINE("POPS GF@%stmp2") &&
			ADD_LINE("PUSHS GF@%stmp2") &&
			ADD_LINE("PUSHS GF@%stmp1") &&
			ADD_LINE("LTS") &&
			ADD_LINE("PUSHS GF@%stmp2") &&
			ADD_LINE("PUSHS GF@%stmp1") &&
			ADD_LINE("EQS") &&
			ADD_LINE("ORS");
			break;
		case TypeGreaterEq:
			ret =
			ADD_LINE("POPS GF@%stmp1") &&
			ADD_LINE("POPS GF@%stmp2") &&
			ADD_LINE("PUSHS GF@%stmp2") &&
			ADD_LINE("PUSHS GF@%stmp1") &&
			ADD_LINE("GTS") &&
			ADD_LINE("PUSHS GF@%stmp2") &&
			ADD_LINE("PUSHS GF@%stmp1") &&
			ADD_LINE("EQS") &&
			ADD_LINE("ORS");
			break;
		default:
			return false;
			break;
	}
	return ret;
}

bool generate_operation_concat()
{
	return
		ADD_LINE("POPS GF@%stmp1") &&
		ADD_LINE("POPS GF@%stmp2") &&
		ADD_LINE("CONCAT GF@%stmp2 GF@%stmp2 GF@%stmp1") &&
		ADD_LINE("PUSHS GF@%stmp2");
}

bool generate_operation_unspecified(char* label, Type operation)
{
	bool ret =
	ADD_LINE("POPS GF@%stmp1") &&
	ADD_LINE("POPS GF@%stmp2") &&
	ADD_LINE("TYPE GF@%stmp1%type GF@%stmp1") &&
	ADD_LINE("TYPE GF@%stmp2%type GF@%stmp2");

	if(operation == TypeOperatorFloorDiv)
	{
		ret &=
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$fint GF@%stmp2%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$ffloat GF@%stmp2%type string@float\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$ffloat\n") &&
		ADD_LINE("FLOAT2INT GF@%stmp2 GF@%stmp2") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("fint\n") &&

		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$sint GF@%stmp1%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$sfloat GF@%stmp1%type string@float\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$sfloat\n") &&
		ADD_LINE("FLOAT2INT GF@%stmp1 GF@%stmp1") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("sint\n");
	}
	else if(operation == TypeOperatorDiv)
	{
		ret &=
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$fint GF@%stmp2%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$ffloat GF@%stmp2%type string@float\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$fint\n") &&
		ADD_LINE("INT2FLOAT GF@%stmp2 GF@%stmp2") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("ffloat\n") &&

		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$sint GF@%stmp1%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$sfloat GF@%stmp1%type string@float\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$sint\n") &&
		ADD_LINE("INT2FLOAT GF@%stmp1 GF@%stmp1") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("sfloat\n");
	}
	else
	{
		ret &=
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$same") && ADD_CODE(" GF@%stmp1%type GF@%stmp2%type\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$fint") && ADD_CODE(" GF@%stmp2%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$ffloat") && ADD_CODE(" GF@%stmp2%type string@float\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$fint\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$fint_sfloat") && ADD_CODE(" GF@%stmp1%type string@float\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$fint_sfloat\n") &&
		ADD_LINE("FLOAT2INT GF@%stmp1 GF@%stmp1") &&
		ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("$same\n") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$ffloat\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$ffloat_sint") && ADD_CODE(" GF@%stmp1%type string@int\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$ffloat_sint\n") &&
		ADD_LINE("INT2FLOAT GF@%stmp1 GF@%stmp1") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$same\n");
	}

	ret &=
	ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("$concat") && ADD_CODE(" GF@%stmp1%type string@string\n") &&
	ADD_LINE("PUSHS GF@%stmp2") &&
	ADD_LINE("PUSHS GF@%stmp1") &&
	generate_operation(operation) &&
	ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("$end\n") &&
	ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$concat\n");
	if(operation == TypeOperatorPlus)
	{
		ret &=
		ADD_LINE("CONCAT GF@%stmp2 GF@%stmp2 GF@%stmp1") &&
		ADD_LINE("PUSHS GF@%stmp2");
	}
	else
	{
		ret &=
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n");
	}

	ret &= ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("$end\n");

	return ret;
}

bool generate_operation_retype_first_int2float()
{
	return
		ADD_LINE("INT2FLOATS");
}
bool generate_operation_retype_first_float2int()
{
	return
		ADD_LINE("FLOAT2INTS");
}
bool generate_operation_retype_sec_int2float()
{
	return
		ADD_LINE("POPS GF@%stmp1") &&
		ADD_LINE("INT2FLOATS") &&
		ADD_LINE("PUSHS GF@%stmp1");
}
bool generate_operation_retype_sec_float2int()
{
	return
		ADD_LINE("POPS GF@%stmp1") &&
		ADD_LINE("FLOAT2INTS") &&
		ADD_LINE("PUSHS GF@%stmp1");
}

bool generate_pop_var(char* label, bool scope)
{
	return
		(scope ? ADD_CODE("POPS GF@") : ADD_CODE("POPS LF@")) && ADD_CODE(label) && ADD_CODE("\n");
}

bool generate_pop_exp()
{
	return
		ADD_LINE("POPS GF@%exp_result");
}

bool generate_pop_return()
{
	return
		ADD_LINE("POPS LF@%retval");
}

bool generate_if_begin(char* label, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return get_str_from_index(tmp, index) &&
		ADD_LINE("# If BEGIN") &&
		ADD_LINE("TYPE GF@%exp_result%type GF@%exp_result") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%begin GF@%exp_result%type string@bool\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%int GF@%exp_result%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%float GF@%exp_result%type string@float\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%string GF@%exp_result%type string@string\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&//nil

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%string\n") &&
		ADD_LINE("STRLEN GF@%exp_result GF@%exp_result") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%int\n") &&
		ADD_LINE("EQ GF@%exp_result GF@%exp_result int@0") &&
		ADD_LINE("NOT GF@%exp_result GF@%exp_result") &&
		ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("%begin\n") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%float\n") &&
		ADD_LINE("EQ GF@%exp_result GF@%exp_result float@0x0p+0") &&
		ADD_LINE("NOT GF@%exp_result GF@%exp_result") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%begin\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE(tmp) &&  ADD_CODE(" GF@%exp_result bool@false\n");
}
bool generate_else(char* label, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	char tmp2[MAX_DIGITS_DOUBLE];
	return index > 0 && get_str_from_index(tmp, index) && get_str_from_index(tmp2, index+1) &&
		ADD_CODE("JUMP ") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE(tmp2) && ADD_CODE("\n") &&
		ADD_LINE("# Else") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE(tmp) && ADD_CODE("\n");
}
bool generate_if_end(char* label, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return get_str_from_index(tmp, index) &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE(tmp) && ADD_CODE("\n") &&
		ADD_LINE("# If END");
}

bool generate_while_begin(char* label)
{
	return
		ADD_LINE("# While BEGIN") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%begin\n");
}
bool generate_while_loop(char* label)
{
	return
		ADD_LINE("TYPE GF@%exp_result%type GF@%exp_result") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%loop GF@%exp_result%type string@bool\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%int GF@%exp_result%type string@int\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%float GF@%exp_result%type string@float\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%string GF@%exp_result%type string@string\n") &&
		ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&//nil

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%string\n") &&
		ADD_LINE("STRLEN GF@%exp_result GF@%exp_result") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%int\n") &&
		ADD_LINE("EQ GF@%exp_result GF@%exp_result int@0") &&
		ADD_LINE("NOT GF@%exp_result GF@%exp_result") &&
		ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("%loop\n") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%float\n") &&
		ADD_LINE("EQ GF@%exp_result GF@%exp_result float@0x0p+0") &&
		ADD_LINE("NOT GF@%exp_result GF@%exp_result") &&

		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%loop\n") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%end") && ADD_CODE(" GF@%exp_result bool@false\n");
}
bool generate_while_end(char* label)
{
	return
		ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("%begin\n") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%end\n") &&
		ADD_LINE("# While END");
}

bool generate_fnc_begin(char* label)
{
	inside_fnc = true;
	return
		ADD_CODE("\n# Function ") && ADD_CODE(label) && ADD_CODE(" BEGIN\n") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("\n") &&
		ADD_LINE("PUSHFRAME") &&
		ADD_LINE("DEFVAR LF@%retval") &&
		ADD_LINE("MOVE LF@%retval nil@nil");
}

bool generate_fnc_param_get(char* label, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return get_str_from_index(tmp, index) &&
		ADD_CODE("DEFVAR LF@") && ADD_CODE(label) && ADD_CODE("\n") &&
		ADD_CODE("MOVE LF@") && ADD_CODE(label) && ADD_CODE(" LF@%") && ADD_CODE(tmp) && ADD_CODE("\n");
}

bool generate_fnc_return_set_data(Type type, void* data)
{
	return
		ADD_CODE("MOVE LF@%retval ") && generate_var_value(type, data) && ADD_CODE("\n");
}
bool generate_fnc_return_set_var(char* label, bool scope)
{
	return
		(scope ? ADD_CODE("MOVE LF@%retval GF@") : ADD_CODE("MOVE LF@%retval LF@")) && ADD_CODE(label) && ADD_CODE("\n");
}

bool generate_fnc_end(char* label)
{
	bool ret =
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%return\n") &&
		ADD_LINE("POPFRAME") &&
		ADD_LINE("RETURN") &&
		ADD_CODE("# Function ") && ADD_CODE(label) && ADD_CODE(" END\n");
	inside_fnc = false;
	return ret;
}

bool generate_fnc_pre_param()
{
	return
		ADD_LINE("CREATEFRAME");
}
bool generate_fnc_param_set_data(Type type, void* data, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return get_str_from_index(tmp, index) &&
		ADD_CODE("DEFVAR TF@%") && ADD_CODE(tmp) && ADD_CODE("\n") &&
		ADD_CODE("MOVE TF@%") && ADD_CODE(tmp) && ADD_CODE(" ") && generate_var_value(type, data) && ADD_CODE("\n");
}
bool generate_fnc_param_set_var(char* label, bool scope, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return get_str_from_index(tmp, index) &&
		ADD_CODE("DEFVAR TF@%") && ADD_CODE(tmp) && ADD_CODE("\n") &&
		ADD_CODE("MOVE TF@%") && ADD_CODE(tmp) && (scope ? ADD_CODE(" GF@") : ADD_CODE(" LF@")) && ADD_CODE(label) && ADD_CODE("\n");
}
bool generate_fnc_call(char* label)
{
	return
		ADD_CODE("CALL $") && ADD_CODE(label) && ADD_CODE("\n") &&
		ADD_LINE("MOVE GF@%fnc_return TF@%retval");
}

bool generate_fnc_return_get(char* label, bool scope)
{
	return
		(scope ? ADD_CODE("MOVE GF@") : ADD_CODE("MOVE LF@")) && ADD_CODE(label) && ADD_CODE(" GF@%fnc_return\n");
}
