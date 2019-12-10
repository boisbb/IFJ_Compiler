/* generator.h
// název projektu: Implementace překladače imperativního jazyka IFJ19
// tým:            040
// varianta:       II (implementace pomocí tabulky s rozptýlenými položkami)
// autoři:         Boris Burkalo, xburka00, 2BIT
//                 Jiří Herrgott, xherrg00, 2BIT
//                 Jan Klusáček, xklusa14, 2BIT
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdbool.h>
#include "scanner.h"

//label must be unique for each var, if, while, ...
//scope -> 0 local, 1 global
//return true on success, false on fail

typedef enum
{
    LABEL_IF,
    LABEL_WHILE,
    LABEL_CONTROL
} LabelType;

#define MAX_DIGITS_DOUBLE 50//neni presny, ale cca

bool generate_unique_label(char* dest, LabelType type);

bool generator_init();
void generator_free();

char *generator_code_get();
void generator_code_clear();

bool generate_main_begin();
bool generate_main_end();

// Define var fo future use
bool generate_var_declaration(char* label, bool scope);
// Define var on pos
bool generate_var_declaration_on_pos(char* label, bool scope, size_t pos);
// Set var data
bool generate_var_definition(char* label, bool scope, Type type, void* data);

// Push data to stack
bool generate_push_data(Type type, void* data);
// Push var to stack
bool generate_push_var(char* label, bool scope);
// Generate type control then push var to stack
bool generate_push_var_unspecified(char* var_label, bool scope, Type type);
// Do operation on stack
bool generate_operation(Type operation);
// Call instead of generate_operation when both operands are TypeUnspecified
bool generate_operation_unspecified(Type operation);
// Concat two strings on stack
bool generate_operation_concat();
// Retype value on stack top from int to float
bool generate_operation_retype_first_int2float();
// Retype value on stack top from float to int
bool generate_operation_retype_first_float2int();
// Retype value second value on stack from int to float
bool generate_operation_retype_sec_int2float();
// Retype value second value on stack from float to int
bool generate_operation_retype_sec_float2int();
// Pops result to var
bool generate_pop_var(char* label, bool scope);
// Pops result for if and while condition
bool generate_pop_exp();
// Pops result to return value
bool generate_pop_return();

bool generate_if_begin(char* label, size_t* pos);
bool generate_else(char* label);
bool generate_if_end(char* label, bool has_else);

bool generate_while_begin(char* label, size_t* pos);
bool generate_while_loop(char* label);
bool generate_while_end(char* label);

bool generate_fnc_begin(char* label);
// Get param from index
bool generate_fnc_param_get(char* label, unsigned index);
// Set return var data
bool generate_fnc_return_set_data(Type type, void* data);
// Set return var as var
bool generate_fnc_return_set_var(char* label, bool scope);
bool generate_fnc_return(char* label);
bool generate_fnc_end(char* label);

// Must be called before param_set or call
bool generate_fnc_pre_param();
// Set param data with index
bool generate_fnc_param_set_data(Type type, void* data, unsigned index);
// Set param var with index
bool generate_fnc_param_set_var(char* label, bool scope, unsigned index);
bool generate_fnc_call(char* label);
// Save your return value to var
bool generate_fnc_return_get(char* label, bool scope);


/* EXAMPLE PROGRAM
generator_init();

generate_fnc_begin("soucet");
generate_fnc_param_get("s1", 0);
generate_fnc_param_get("s2", 1);
generate_var_declaration("vys", 0);
generate_push_var("s1", 0);
generate_push_var("s2", 0);
generate_operation_unspecified("soucet$type_control", TypeOperatorPlus);
generate_pop_var("vys", 0);
generate_fnc_return_set_var("vys", 0);
generate_fnc_end("soucet");

generate_main_begin();

generate_var_declaration("param1", 0);
generate_var_declaration("param2", 0);
generate_var_declaration("vys", 0);

generate_fnc_pre_param();
generate_fnc_call("inputi");
generate_fnc_return_get("param1", 0);

generate_fnc_pre_param();
generate_fnc_call("inputi");
generate_fnc_return_get("param2", 0);

generate_fnc_pre_param();
generate_fnc_param_set_var("param1", 0, 0);
generate_fnc_param_set_var("param2", 0, 1);
generate_fnc_call("soucet");
generate_fnc_return_get("vys", 0);

generate_fnc_pre_param();
generate_push_var("vys", 0);
int i = 1;
generate_fnc_param_set_data(TypeInt, &i, 0);
generate_fnc_call("print");

generate_main_end();

printf("%s", generator_code_get());

generator_free();
*/


#endif
