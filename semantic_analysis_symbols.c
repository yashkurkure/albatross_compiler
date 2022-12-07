#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "semantic_analysis_symbols.h"

// Used for code and variables outside of functions
static frame global_empty_frame;

void symbolResolutionExpr(exp_node * e, S_table global_types, S_table function_rets, frame * f) {
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);

    if(!e) return;
    switch(e->kind){
        case int_exp: {
            return; // No variables to be checked in a constant expression
        }
        case string_exp: {
            return; // No variables to be checked in a constant expression
        }
        case binop_exp: {
            symbolResolutionExpr(e->data.bin_ops.e1, global_types, function_rets, f);
            symbolResolutionExpr(e->data.bin_ops.e2, global_types, function_rets, f);
            return;
        }
        case call_exp: {
            if (!S_look(function_rets, S_Symbol(e->data.call_ops.name))) {
                fprintf(stderr,"Called function \"%s\" without declaring it.\n", e->data.call_ops.name);
                exit(3);
            }
            list * l = e->data.call_ops.args;
            while (l != NULL) {
                exp_node * arg = (exp_node*) l->head;
                symbolResolutionExpr(arg, global_types, function_rets, f);
                l = l->next;
            }

            return;
        }
        case unop_exp: {
            symbolResolutionExpr(e->data.un_ops.e, global_types, function_rets, f);
            return;
        }
        case var_exp: {
            if (!S_look(global_types, S_Symbol(e->data.var_ops.name))) {
                if (!S_look(f->args_locs_types, S_Symbol(e->data.var_ops.name))) {
                    fprintf(stderr,"Read variable \"%s\" without declaring it.\n", e->data.var_ops.name);
                    exit(3);
                }
            }
            return;
        }
        default:
            assert(0); // Dead code
    }
}

void symbolResolutionStmts(list * l, S_table locals, S_table function_rets, frame * f);

void symbolResolutionStmt(stmt_node * s, S_table global_types, S_table function_rets, frame * f) {
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);
    if(!s) return;
    switch(s->kind){
        case assign_stmt: {
            if (!S_look(global_types, S_Symbol(s->data.assign_ops.lhs))) {
                if (!S_look(f->args_locs_types, S_Symbol(s->data.assign_ops.lhs))) {
                    fprintf(stderr, "Written variable \"%s\" without declaring it.\n", s->data.assign_ops.lhs);
                    exit(3);
                }
            }
            symbolResolutionExpr(s->data.assign_ops.rhs, global_types, function_rets, f);
            return;
        }
        case if_stmt: {
            symbolResolutionExpr(s->data.if_ops.cond, global_types, function_rets, f);
            symbolResolutionStmts(s->data.if_ops.then_stmts, global_types, function_rets, f);
            symbolResolutionStmts(s->data.if_ops.else_stmts, global_types, function_rets, f);
            return;
        }
        case while_stmt: {
            symbolResolutionExpr(s->data.while_ops.cond, global_types, function_rets, f);
            symbolResolutionStmts(s->data.while_ops.body, global_types, function_rets, f);
            symbolResolutionStmts(s->data.while_ops.otherwise, global_types, function_rets, f);
            return;
        }
        case repeat_stmt: {
            symbolResolutionExpr(s->data.repeat_ops.cond, global_types, function_rets, f);
            symbolResolutionStmts(s->data.repeat_ops.body, global_types, function_rets, f);
            return;
        }
        case ret_stmt: {
            symbolResolutionExpr(s->data.ret_exp, global_types, function_rets, f);
            return;
        }
        case call_stmt: {
            if (!S_look(function_rets, S_Symbol(s->data.call_ops.name))) {
                fprintf(stderr,"Called function %s without declaring it.\n", s->data.call_ops.name);
                exit(3);
            }
            list * l = s->data.call_ops.args;
            while (l != NULL) {
                exp_node * arg = (exp_node*) l->head;
                symbolResolutionExpr(arg, global_types, function_rets, f);
                l = l->next;
            }
            return;
        }
        default:
            assert(0); // Dead code
    }
}

void symbolResolutionStmts(list * l, S_table globals_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    symbolResolutionStmt(l->head, globals_types, function_rets, f);
    symbolResolutionStmts(l->next, globals_types, function_rets, f);
}

void symbolResolutionVariable(vardec_node * vardec, S_table globals_types, S_table function_rets, frame * f) {
    // TODO figure out if this is a global variable or a local variable
    // TODO Hint:  to start, just assume that everything is a global variable

    if (S_look(globals_types, S_Symbol(vardec->name))) {
        fprintf(stderr,"Attempting to define duplicate global variable %s.\n", vardec->name);
        exit(3);
    }
    symbolResolutionExpr(vardec->init, globals_types, function_rets, f);

    S_enter(globals_types, S_Symbol(vardec->name), vardec->type);

    // TODO check that all symbols in the variable initialization are known to fail negative tests
    return;

    UNUSED(function_rets);
    UNUSED(f);
}

void symbolResolutionVariables(list * l, S_table global_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    symbolResolutionVariable((vardec_node *)l->head, global_types, function_rets, f);
    symbolResolutionVariables(l->next, global_types, function_rets, f);
}

void symbolResolutionFunction(fundec_node * fundec, S_table globals, S_table functions_rets, frame * f) {
    printf("SYMBOL RES FUN\n");
    // TODO add each argument type to the f->args_locs_types
    list * l = fundec->args;
    while (l != NULL) {
        param * p = (param*) l->head;
        if (S_look(globals, S_Symbol(p->name))) {
            fprintf(stderr,"Attempting to define argument with same name as global variable \"%s\".\n", p->name);
            exit(3);
        }
        printf("Added arg %s to args_locs for func %s\n", p->name, fundec->name);
        S_enter(f->args_locs_types, S_Symbol(p->name), p->ty);
        l = l->next;
    }

    // TODO add each local variable type to f->args_locs_types
    l = fundec->locs;
    while (l != NULL) {
        vardec_node * dec = (vardec_node*) l->head;
        if (S_look(globals, S_Symbol(dec->name))) {
            fprintf(stderr,"Attempting to define local variable with same name as global variable \"%s\".\n", dec->name);
            exit(3);
        }
        if (S_look(f->args_locs_types, S_Symbol(dec->name))) {
            fprintf(stderr,"Attempting to define local variable with same name as argument \"%s\".\n", dec->name);
            exit(3);
        }
        printf("Added loc %s to args_locs for func %s\n", dec->name, fundec->name);
        S_enter(f->args_locs_types, S_Symbol(dec->name), dec->type);
        l = l->next;
    }

    // TODO check that the statements in the body of the function only use known variables and functions

    S_enter(functions_rets, S_Symbol(fundec->name), fundec);
    symbolResolutionStmts(fundec->stmts, globals, functions_rets, f);
    return;
}

void symbolResolutionFunctions(list * l, S_table global_types, S_table function_ret_types, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;

    if (S_look(function_ret_types, S_Symbol(fundec->name))) {
        fprintf(stderr,"Attempting to define duplicate function %s.\n", fundec->name);
        exit(3);
    }

    // Create a new frame
    frame * f = malloc(sizeof(frame));

    // Initialize all memory so we can read it later
    f->args_locs_types = S_empty();
    f->indexes = S_empty();
    f->ret = NULL;

    // Add newly computed frame to frames
    S_enter(frames, S_Symbol(fundec->name), f);

    symbolResolutionFunction(fundec, global_types, function_ret_types, f);
    symbolResolutionFunctions(l->next, global_types, function_ret_types, frames);
}

static void registerIntrinsics(S_table function_rets) {
    fundec_node * f;
    f = FunDecNode("exit", VoidTyNode(), ListAddFirst(Param(IntTyNode(), "n"), NULL), NULL, NULL);
    S_enter(function_rets, S_Symbol("exit"), f);

    f = FunDecNode("printint", VoidTyNode(), ListAddFirst(Param(IntTyNode(), "n"), NULL), NULL,
                   NULL);
    S_enter(function_rets, S_Symbol("printint"), f);

    f = FunDecNode("printstring", VoidTyNode(), ListAddFirst(Param(StringTyNode(), "s"), NULL),
                   NULL,NULL);
    S_enter(function_rets, S_Symbol("printstring"), f);
}

void symbolResolution(program * p, S_table global_types, S_table function_rets, S_table frames) {

    registerIntrinsics(function_rets);
    global_empty_frame.args_locs_types = S_empty();
    global_empty_frame.indexes = S_empty();

    symbolResolutionVariables(p->variables, global_types, function_rets, &global_empty_frame);
    symbolResolutionFunctions(p->functions, global_types, function_rets, frames);
    symbolResolutionStmts(p->statements, global_types, function_rets, &global_empty_frame);
}
