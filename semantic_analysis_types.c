#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "semantic_analysis_types.h"

ty_node * typeCheckExpr(exp_node * e, S_table global_types, S_table function_rets, frame * f) {
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);

    switch(e->kind){
        case int_exp: {
            return IntTyNode();
        }
        case string_exp: {
            return StringTyNode();
        }
        case binop_exp: {
            ty_node * expected = IntTyNode();
            ty_node * left = typeCheckExpr(e->data.bin_ops.e1, global_types, function_rets, f);
            ty_node * right = typeCheckExpr(e->data.bin_ops.e2, global_types, function_rets, f);

            if (left->kind != expected->kind) {
                fprintf(stderr,"Bad left type on binary operation.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(left));
                exit(3);
            } else if (right->kind != expected->kind) {
                fprintf(stderr,"Bad right type on binary operation.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(right));
                exit(3);
            }

            return expected;
        }
        case call_exp: {
            fundec_node * fundec = (fundec_node*) S_look(function_rets, S_Symbol(e->data.call_ops.name));

            list * formal_args = fundec->args;
            list * actual_args = e->data.call_ops.args;
            int i = 0;

            while (formal_args != NULL && actual_args != NULL) {
                ty_node * fa = ((param *) formal_args->head)->ty;
                ty_node * aa = typeCheckExpr(actual_args->head, global_types, function_rets, f);

                if (fa->kind != aa->kind) {
                    fprintf(stderr,"Bad type for argument %d when calling function \"%s\".  Expected %s, but got %s\n", i, e->data.call_ops.name, typeToStr(fa), typeToStr(aa));
                    exit(3);
                }

                formal_args = formal_args->next;
                actual_args = actual_args->next;
                i += 1;
            }

            if (formal_args != NULL || actual_args != NULL) {
                fprintf(stderr,"Wrong number of arguments when calling function \"%s\"\n", e->data.call_ops.name);
                exit(3);
            }


            return fundec->type;
        }
        case unop_exp: {
            ty_node * expected = IntTyNode();
            ty_node * op = typeCheckExpr(e->data.un_ops.e, global_types, function_rets, f);

            if (op->kind != expected->kind) {
                fprintf(stderr,"Bad left type on unary operation.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(op));
                exit(3);
            }

            return expected;
        }
        case var_exp: {
            ty_node * type = S_look(global_types, S_Symbol(e->data.var_ops.name));
            if (type == NULL) {
                type = S_look(f->args_locs_types, S_Symbol(e->data.var_ops.name));
            }
            assert(type);
            return type;
        }
        default:
            assert(0); // Dead code
    }
}

void typeCheckStmts(list * l, S_table locals, S_table function_rets, frame * f);

void typeCheckStmt(stmt_node * s, S_table global_types, S_table function_rets, frame * f) {
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);
    if(!s) return;
    switch(s->kind){
        case assign_stmt: {
            ty_node * expected = S_look(global_types, S_Symbol(s->data.assign_ops.lhs));
            if (expected == NULL) {
                expected = S_look(f->args_locs_types, S_Symbol(s->data.assign_ops.lhs));
            }
            assert(expected);
            ty_node * actual = typeCheckExpr(s->data.assign_ops.rhs, global_types, function_rets, f);

            if (expected->kind != actual->kind) {
                fprintf(stderr,"Bad type in variable assignment.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(actual));
                exit(3);
            }

            return;
        }
        case if_stmt: {
            ty_node * type = typeCheckExpr(s->data.if_ops.cond, global_types, function_rets, f);
            ty_node * expected = IntTyNode();

            if (type->kind != expected->kind) {
                fprintf(stderr,"Bad if condition type.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(type));
                exit(3);
            }

            typeCheckStmts(s->data.if_ops.then_stmts, global_types, function_rets, f);
            typeCheckStmts(s->data.if_ops.else_stmts, global_types, function_rets, f);
            return;
        }
        case while_stmt: {
            ty_node * type = typeCheckExpr(s->data.while_ops.cond, global_types, function_rets, f);
            ty_node * expected = IntTyNode();

            if (type->kind != expected->kind) {
                fprintf(stderr,"Bad while guard type.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(type));
                exit(3);
            }

            typeCheckStmts(s->data.while_ops.body, global_types, function_rets, f);
            typeCheckStmts(s->data.while_ops.otherwise, global_types, function_rets, f);
            return;
        }
        case repeat_stmt: {
            ty_node * type = typeCheckExpr(s->data.repeat_ops.cond, global_types, function_rets, f);
            ty_node * expected = IntTyNode();

            if (type->kind != expected->kind) {
                fprintf(stderr,"Bad repeat type.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(type));
                exit(3);
            }

            typeCheckStmts(s->data.repeat_ops.body, global_types, function_rets, f);
            return;
        }
        case ret_stmt: {
            ty_node * expected;

            if (f == NULL) {
                expected = IntTyNode(); // TODO check if we are inside a function, get the return type of the function
            } else {
                expected = f->ret;
            }

            ty_node * type;

            if (s->data.ret_exp == NULL) {
                type = VoidTyNode();
            } else {
                type = typeCheckExpr(s->data.ret_exp, global_types, function_rets, f);
            }

            if (type->kind != expected->kind) {
                fprintf(stderr,"Bad return type.  Expected %s, but got %s\n", typeToStr(expected), typeToStr(type));
                exit(3);
            }

            return;
        }
        case call_stmt: {
            fundec_node * fundec = (fundec_node*) S_look(function_rets, S_Symbol(s->data.call_ops.name));

            list * formal_args = fundec->args;
            list * actual_args = s->data.call_ops.args;
            int i = 0;

            while (formal_args != NULL && actual_args != NULL) {
                ty_node * fa = ((param *) formal_args->head)->ty;
                ty_node * aa = typeCheckExpr(actual_args->head, global_types, function_rets, f);

                if (fa->kind != aa->kind) {
                    fprintf(stderr,"Bad type for argument %d when calling function \"%s\".  Expected %s, but got %s\n", i, s->data.call_ops.name, typeToStr(fa), typeToStr(aa));
                    exit(3);
                }

                formal_args = formal_args->next;
                actual_args = actual_args->next;
                i += 1;
            }

            if (formal_args != NULL || actual_args != NULL) {
                fprintf(stderr,"Wrong number of arguments when calling function \"%s\"\n", s->data.call_ops.name);
                exit(3);
            }
            return;
        }
        default:
            assert(0); // Dead code
    }
}

void typeCheckStmts(list * l, S_table globals_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    typeCheckStmt(l->head, globals_types, function_rets, f);
    typeCheckStmts(l->next, globals_types, function_rets, f);
}

void typeCheckVariable(vardec_node * vardec, S_table globals_types, S_table function_rets, frame * f) {
    // TODO ensure that variable initialization expression has the same type as the variable definition

    ty_node * expected = vardec->type;
    ty_node * actual = typeCheckExpr(vardec->init, globals_types, function_rets, f);

    if (expected->kind != actual->kind) {
        fprintf(stderr,"Bad type when initializing variable %s.  Expected %s, but got %s\n", vardec->name, typeToStr(expected), typeToStr(actual));
        exit(3);
    }

    return;

    UNUSED(globals_types);
    UNUSED(function_rets);
    UNUSED(f);
}

void typeCheckVariables(list * l, S_table global_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    typeCheckVariable((vardec_node *)l->head, global_types, function_rets, f);
    typeCheckVariables(l->next, global_types, function_rets, f);
}

void typeCheckFunction(fundec_node * fundec, S_table globals_types, S_table functions_rets, frame * f) {
    // TODO ensure that the body of the function is well typed
    // TODO ensure local variables are initialized with the correct type

    typeCheckVariables(fundec->locs, globals_types, functions_rets, f);

    f->ret = fundec->type;
    typeCheckStmts(fundec->stmts, globals_types, functions_rets, f);
    return;

    UNUSED(globals_types);
    UNUSED(functions_rets);
}

void typeCheckFunctions(list * l, S_table global_types, S_table function_ret_types, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    frame * f = S_look(frames, S_Symbol(fundec->name));
    assert(f);
    typeCheckFunction(fundec, global_types, function_ret_types, f);
    typeCheckFunctions(l->next, global_types, function_ret_types, frames);
}

void typeCheck(program * p, S_table global_types, S_table function_rets, S_table frames) {
    typeCheckVariables(p->variables, global_types, function_rets, NULL);
    typeCheckFunctions(p->functions, global_types, function_rets, frames);
    typeCheckStmts(p->statements, global_types, function_rets, NULL);
}