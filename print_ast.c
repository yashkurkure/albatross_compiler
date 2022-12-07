#include <stdio.h>
#include <assert.h>
#include "ast.h"
#include "print_ast.h"

void printSymbolsNamesTypesExpr(exp_node * e, S_table global_types, S_table function_rets, frame * f) {
    if(!e) return;
    switch(e->kind){
        case int_exp: {
            return;
        }
        case string_exp: {
            return;
        }
        case binop_exp: {
            printSymbolsNamesTypesExpr(e->data.bin_ops.e1, global_types, function_rets, f);
            printSymbolsNamesTypesExpr(e->data.bin_ops.e2, global_types, function_rets, f);
            break;
        }
        case call_exp: {
            fundec_node * ret = S_look(function_rets, S_Symbol(e->data.call_ops.name));
            assert(ret);
            printf("Function called \"%s\" returns %s\n", e->data.call_ops.name, typeToStr(ret->type));
            break;
        }
        case unop_exp: {
            printSymbolsNamesTypesExpr(e->data.un_ops.e, global_types, function_rets, f);
            break;
        }
        case var_exp: {
            ty_node * type = (ty_node *) S_look(global_types, S_Symbol(e->data.var_ops.name));
            if (type == NULL) {
                assert(f);
                type = (ty_node *) S_look(f->args_locs_types, S_Symbol(e->data.var_ops.name));
                assert(type);
                long idx = (long) S_look(f->indexes, S_Symbol(e->data.var_ops.name));
                printf("Argument/local read \"%s\" type %s frame position %ld\n", e->data.var_ops.name, typeToStr(type), idx);
            } else {
                printf("Variable read \"%s\" type %s\n", e->data.var_ops.name, typeToStr(type));
            }
            break;
        }
        default:
            assert(0);
    }
}

void printSymbolsNamesTypesStmts(list * l, S_table locals, S_table function_rets, frame * f);

void printSymbolsNamesTypesStmt(stmt_node * s, S_table global_types, S_table function_rets, frame * f) {
    if(!s) return;
    switch(s->kind){
        case assign_stmt: {
            printSymbolsNamesTypesExpr(s->data.assign_ops.rhs, global_types, function_rets, f);
            ty_node * type = (ty_node *) S_look(global_types, S_Symbol(s->data.assign_ops.lhs));
            if (type == NULL) {
                type = (ty_node *) S_look(f->args_locs_types, S_Symbol(s->data.assign_ops.lhs));
                assert(type);
                long idx = (long) S_look(f->indexes, S_Symbol(s->data.assign_ops.lhs));
                printf("Argument/local written \"%s\" type %s frame position %ld\n", s->data.assign_ops.lhs, typeToStr(type), idx);
            } else {
                printf("Variable written \"%s\" type %s\n", s->data.assign_ops.lhs, typeToStr(type));
            }
            break;
        }
        case if_stmt: {
            printSymbolsNamesTypesExpr(s->data.if_ops.cond, global_types, function_rets, f);
            printSymbolsNamesTypesStmts(s->data.if_ops.then_stmts, global_types, function_rets, f);
            printSymbolsNamesTypesStmts(s->data.if_ops.else_stmts, global_types, function_rets, f);
            break;
        }
        case while_stmt: {
            printSymbolsNamesTypesExpr(s->data.while_ops.cond, global_types, function_rets, f);
            printSymbolsNamesTypesStmts(s->data.while_ops.body, global_types, function_rets, f);
            printSymbolsNamesTypesStmts(s->data.while_ops.otherwise, global_types, function_rets, f);
            break;
        }
        case repeat_stmt: {
            printSymbolsNamesTypesExpr(s->data.repeat_ops.cond, global_types, function_rets, f);
            printSymbolsNamesTypesStmts(s->data.repeat_ops.body, global_types, function_rets, f);
            break;
        }
        case ret_stmt: {
            printSymbolsNamesTypesExpr(s->data.ret_exp, global_types, function_rets, f);
            break;
        }
        case call_stmt: {
            fundec_node * ret = S_look(function_rets, S_Symbol(s->data.call_ops.name));
            assert(ret);
            printf("Function called \"%s\" returns %s\n", s->data.call_ops.name, typeToStr(ret->type));
            break;
        }
        default:
            assert(0);
    }
}

void printSymbolsNamesTypesStmts(list * l, S_table globals_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    printSymbolsNamesTypesStmt(l->head, globals_types, function_rets, f);
    printSymbolsNamesTypesStmts(l->next, globals_types, function_rets, f);
}

void printSymbolsNamesTypesVariable(vardec_node * node, S_table globals_types, S_table function_rets, frame * f) {
    printf("Variable declared \"%s\" type %s\n", node->name, typeToStr(node->type));
    printSymbolsNamesTypesExpr(node->init, globals_types, function_rets, f);
}

void printSymbolsNamesTypesVariables(list * l, S_table global_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    printSymbolsNamesTypesVariable((vardec_node *)l->head, global_types, function_rets, f);
    printSymbolsNamesTypesVariables(l->next, global_types, function_rets, f);
}

void printSymbolsNamesTypesFunction(fundec_node * fundec, S_table globals, S_table functions_rets, frame * f) {
    fundec_node * ff = (fundec_node*) S_look(functions_rets, S_Symbol(fundec->name));
    printf("Function declared \"%s\" returns %s\n", fundec->name, typeToStr(ff->type));

    list * loc = fundec->args;

    while (loc != NULL) {
        assert(f);
        param * p = (param *) loc->head;
        ty_node * ty = S_look(f->args_locs_types, S_Symbol(p->name));
        assert(ty);
        long idx = (long) S_look(f->indexes, S_Symbol(p->name));
        printf("\tArgument \"%s\" type %s position %ld\n", p->name, typeToStr(ty), idx);
        loc = loc->next;
    }

    loc = fundec->locs;

    while (loc != NULL) {
        assert(f);
        vardec_node * p = (vardec_node *) loc->head;
        ty_node * ty = S_look(f->args_locs_types, S_Symbol(p->name));
        assert(ty);
        long idx = (long) S_look(f->indexes, S_Symbol(p->name));
        printf("\tLocal variable \"%s\" type %s position %ld\n", p->name, typeToStr(ty), idx);
        printSymbolsNamesTypesExpr(p->init, globals, functions_rets, f);
        loc = loc->next;
    }

    printSymbolsNamesTypesStmts(fundec->stmts, globals, functions_rets, f);
}

void printSymbolsNamesTypesFunctions(list * l, S_table global_types, S_table function_ret_types, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    frame * f = S_look(frames, S_Symbol(fundec->name));
    printSymbolsNamesTypesFunction(fundec, global_types, function_ret_types, f);
    printSymbolsNamesTypesFunctions(l->next, global_types, function_ret_types, frames);
}

// Used for code and variables outside of functions
static frame global_empty_frame;

void printSymbolsNamesTypes(program * p, S_table global_types, S_table function_rets, S_table frames) {

    global_empty_frame.args_locs_types = S_empty();
    global_empty_frame.indexes = S_empty();

    printSymbolsNamesTypesVariables(p->variables, global_types, function_rets, &global_empty_frame);
    printSymbolsNamesTypesFunctions(p->functions, global_types, function_rets, frames);
    printSymbolsNamesTypesStmts(p->statements, global_types, function_rets, &global_empty_frame);
}
