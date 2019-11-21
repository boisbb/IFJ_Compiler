#ifndef ERROR_H
#define ERROR_H

#if defined(DEBUG) && DEBUG > 0
 #include <stdio.h> //not all files has included fprintf
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#define NO_ERROR 0
#define ERROR_LEXICAL 1
#define ERROR_SYNTAX 2
#define ERROR_SEMANTIC 3
#define ERROR_SEMANTIC_RUNTIME 4
#define ERROR_SEMANTIC_FUNCTION_CALL 5
#define ERROR_SEMANTIC_OTHER 6
#define ERROR_DIV_BY_NULL 9
#define ERROR_INTERNAL 99

#endif
