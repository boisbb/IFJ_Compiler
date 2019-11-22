#include "generator.h"


#include "strings.h"
#include "generator_functions.h"

//GLOBALS
String code = {};
/////////

#define MAX_DIGITS_DOUBLE 50

#define ADD_CODE(str) \
	str_push(&code, (str))

#define ADD_LINE(line) \
	ADD_CODE(line "\n")


static inline bool generate_header()
{
	return
		ADD_LINE("# Program BEGIN") &&
		ADD_LINE(".IFJcode19") &&
		ADD_LINE("DEFVAR GF@%fnc_return") &&
		ADD_LINE("DEFVAR GF@%exp_result") &&
		ADD_LINE("DEFVAR GF@%stmp1") &&
		ADD_LINE("DEFVAR GF@%stmp2") &&
		//ADD_LINE("DEFVAR GF@%stmp1%type") &&
		//ADD_LINE("DEFVAR GF@%stmp2%type") &&
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
					ret = sprintf(tmp.content, "%g", *(double*)data) > 0 &&
						ADD_CODE("float@") && ADD_CODE(tmp.content);
				}
				else
				{
					ret = ADD_CODE("float@0.0");
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

static inline bool get_str_from_index(char* str, unsigned index)
{
	return sprintf(str, "%d", index) > 0;
}

bool generator_init()
{
	return
		str_init(&code) &&
		generate_header() &&
		generate_predefined_functions();
}

void generator_free()
{
	str_free(&code);
}

char *generator_code_get()
{
	return code.content;
}

void generator_code_clear()
{
	str_clear(&code);
}

bool generate_main_begin()
{
	return
		ADD_LINE("\n# Main scope BEGIN") &&
		ADD_LINE("LABEL $$main") &&
		ADD_LINE("CREATEFRAME") &&
		ADD_LINE("PUSHFRAME");
}
bool generate_main_end()
{
	return
		ADD_LINE("POPFRAME") &&
		ADD_LINE("CLEARS") &&
		ADD_LINE("# Main scope END");
}

bool generate_var_declaration(char* label, bool scope)
{
	return
		scope ? ADD_CODE("DEFVAR GF@") : ADD_CODE("DEFVAR LF@") &&
		ADD_CODE(label) &&
		ADD_CODE("\n");
}

bool generate_var_definition(char* label, bool scope, Type type, void* data)
{
	return
		scope ? ADD_CODE("MOVE GF@") : ADD_CODE("MOVE LF@") &&
		ADD_CODE(label) &&
		ADD_CODE(" ") &&
		generate_var_value(type, data) &&
		ADD_CODE("\n");
}

bool generate_var_runtime_type_control(char* label, char* var_label, bool scope, Type type) //todo
{
	bool ret;
	switch(type)
	{
		case TypeString:
			ret =
			ADD_CODE("DEFVAR LF@") && ADD_CODE(var_label) && ADD_CODE("$type\n") &&
			ADD_CODE("TYPE LF@") && ADD_CODE(var_label) && ADD_CODE("$type ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("string LF@") && ADD_CODE(var_label) && ADD_CODE("$type string@string\n") &&
			ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("string\n");
			break;
		case TypeInt:
			ret =
			ADD_CODE("DEFVAR LF@") && ADD_CODE(var_label) && ADD_CODE("$type\n") &&
			ADD_CODE("TYPE LF@") && ADD_CODE(var_label) && ADD_CODE("$type ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("int LF@") && ADD_CODE(var_label) && ADD_CODE("$type string@int\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("float LF@") && ADD_CODE(var_label) && ADD_CODE("$type string@float\n") &&
			ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("float\n") &&
			ADD_CODE("FLOAT2INT LF@") && ADD_CODE(label) && ADD_CODE(" ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("end\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("int\n") &&
			ADD_CODE("MOVE LF@") && ADD_CODE(label) && ADD_CODE(" ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("end\n");
			break;
		case TypeFloat:
			ret =
			ADD_CODE("DEFVAR LF@") && ADD_CODE(var_label) && ADD_CODE("$type\n") &&
			ADD_CODE("TYPE LF@") && ADD_CODE(var_label) && ADD_CODE("$type ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("int LF@") && ADD_CODE(var_label) && ADD_CODE("$type string@int\n") &&
			ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("float LF@") && ADD_CODE(var_label) && ADD_CODE("$type string@float\n") &&
			ADD_CODE("EXIT int@") && ADD_CODE(STR(ERROR_SEMANTIC_RUNTIME)) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("int\n") &&
			ADD_CODE("INT2FLOAT LF@") && ADD_CODE(label) && ADD_CODE(" ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("JUMP $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("end\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("float\n") &&
			ADD_CODE("MOVE LF@") && ADD_CODE(label) && ADD_CODE(" ") && scope ? ADD_CODE("GF@") : ADD_CODE("LF@") && ADD_CODE(var_label) && ADD_CODE("\n") &&
			ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("_") && ADD_CODE("end\n");
			break;
		default:
			return false;
			break;
	}
	return ret;
}

bool generate_push_data(Type type, void* data)
{
	return
		ADD_CODE("PUSHS ") && generate_var_value(type, data) && ADD_CODE("\n");
}
bool generate_push_var(char* label, bool scope)
{
	return
		scope ? ADD_CODE("PUSHS GF@") : ADD_CODE("PUSHS LF@") && ADD_CODE(label) && ADD_CODE("\n");
}

bool generate_operation(Type type)
{
	bool ret;/*=
	ADD_LINE("POPS GF@%stmp1") &&
	ADD_LINE("POPS GF@%stmp2") &&
	ADD_LINE("TYPE GF%stmp1%type GF@%stmp1") &&
	ADD_LINE("TYPE GF%stmp2%type GF@%stmp2");*/
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

bool generate_pop_var(char* label, bool scope)
{
	return
		scope ? ADD_CODE("POPS GF@") : ADD_CODE("POPS LF@") && ADD_CODE(label) && ADD_CODE("\n");
}

bool generate_if_begin(char* label, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return get_str_from_index(tmp, index) &&
		ADD_LINE("# If BEGIN") &&
		ADD_CODE("JUMPIFEQ $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE(tmp) &&  ADD_CODE(" GF@%exp_result bool@false\n");
}
bool generate_else(char* label, unsigned index)
{
	char tmp[MAX_DIGITS_DOUBLE];
	return index > 0 && get_str_from_index(tmp, index) &&
		ADD_CODE("JUMP ") && ADD_CODE(label) && ADD_CODE("%0\n") &&
		ADD_LINE("# Else") &&
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%") && ADD_CODE(tmp) && ADD_CODE("\n");
}
bool generate_if_end(char* label)
{
	return
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%0\n") &&
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
		scope ? ADD_CODE("MOVE LF@%retval GF@") : ADD_CODE("MOVE LF@%retval LF@") && ADD_CODE(label) && ADD_CODE("\n");
}

bool generate_fnc_end(char* label)
{
	return
		ADD_CODE("LABEL $") && ADD_CODE(label) && ADD_CODE("%return\n") &&
		ADD_LINE("POPFRAME") &&
		ADD_LINE("RETURN") &&
		ADD_CODE("# Function ") && ADD_CODE(label) && ADD_CODE(" END\n");
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
		ADD_CODE("MOVE TF@%") && ADD_CODE(tmp) && scope ? ADD_CODE(" GF@") : ADD_CODE(" LF@") && ADD_CODE(label) && ADD_CODE("\n");
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
		scope ? ADD_CODE("MOVE GF@") : ADD_CODE("MOVE LF@") && ADD_CODE(label) && ADD_CODE(" GF@%fnc_return\n");
}
