#ifndef SEM_ANALYSIS_TYPES_H
#define SEM_ANALYSIS_TYPES_H

#include "ast.h"

// type checking
void typeCheck(program * p, S_table global_types, S_table function_rets, S_table frames);

#endif
