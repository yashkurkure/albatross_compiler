#ifndef PRINT_AST_H
#define PRINT_AST_H

#include "ast.h"

// print results
void printSymbolsNamesTypes(program * p, S_table global_types, S_table function_rets, S_table frames);

#endif
