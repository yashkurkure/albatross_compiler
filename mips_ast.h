#ifndef MIPS_AST_H
#define MIPS_AST_H

#include "ast.h"

// print results
void mips_ast(program * p, S_table global_types, S_table function_rets, S_table frames, FILE * out);

#endif
