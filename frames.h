#ifndef SEM_ANALYSIS_FRAMES_H
#define SEM_ANALYSIS_FRAMES_H

#include "ast.h"

// compute function frames
void frames(program * p, S_table global_types, S_table function_rets, S_table frames);

#endif
