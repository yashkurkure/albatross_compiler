#ifndef SEM_ANALYSIS_SYM_H
#define SEM_ANALYSIS_SYM_H

#include "ast.h"

// symbol analysis
void symbolResolution(program * p, S_table global_types, S_table function_rets, S_table frames);

#endif
