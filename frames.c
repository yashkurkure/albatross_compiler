#include <stdio.h>
#include "ast.h"
#include "frames.h"
#include "util.h"

void framesFunction(fundec_node * fundec, S_table globals, S_table functions_rets, S_table frames) {
    // Get frame created during symbol resolution
    frame * f = S_look(frames, S_Symbol(fundec->name));

    long i = 0;
    // TODO add each argument position to indexes
    list * l = fundec->args;
    while (l != NULL) {
        param * p = (param*)l->head;
        S_enter(f->indexes, S_Symbol(p->name), (void*)i);
        i += 1;
        l = l->next;
    }

    // TODO add each local variable position to indexes
    l = fundec->locs;
    while (l != NULL) {
        vardec_node * v = (vardec_node*)l->head;
        S_enter(f->indexes, S_Symbol(v->name), (void*)i);
        i += 1;
        l = l->next;
    }

    UNUSED(globals);
    UNUSED(functions_rets);
    UNUSED(f);
}

void framesFunctions(list * l, S_table global_types, S_table function_ret_types, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    framesFunction(fundec, global_types, function_ret_types, frames);
    framesFunctions(l->next, global_types, function_ret_types, frames);
}

void frames(program * p, S_table global_types, S_table function_rets, S_table frames) {
    framesFunctions(p->functions, global_types, function_rets, frames);
}