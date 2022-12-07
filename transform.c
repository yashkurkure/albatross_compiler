#define _POSIX_C_SOURCE 200809L // enable strdup
#include <string.h>

#include <stdio.h>
#include <assert.h>
#include "ast.h"
#include "transform.h"
#include "util.h"

extern program p;

static char * generateFreshGlobalStringVar() {
    static int lastVar = 0;
    static char buffer[1024];

    snprintf(buffer, sizeof(buffer), "_glob_string%d", lastVar);
    lastVar += 1;
    return strdup(buffer);
}

static char * generateFreshGlobalIntVar() {
    static int lastVar = 0;
    static char buffer[1024];

    snprintf(buffer, sizeof(buffer), "_glob_int%d", lastVar);
    lastVar += 1;
    return strdup(buffer);
}


void transformExpr(exp_node * e, S_table global_types, S_table function_rets, frame * f) {
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);
    if(!e) return;
    switch(e->kind){
        case int_exp: {
            break;
        }
        case string_exp: {
            // Create a fresh global variable of type string
            char * name = generateFreshGlobalStringVar();
            // Initialize that global variable with the contents of the string
            p.variables = ListAddFirst(
                    VarDecNode(name, StringTyNode(), StringNode(e->data.sval)),
                    p.variables);
            // Register the new global variable in the right table
            S_enter(global_types, S_Symbol(name), StringTyNode());
            // Replace the string_exp with a var_exp for that fresh global variable
            e->kind = var_exp;
            e->data.var_ops.name = name;
            break;
        }
        case binop_exp: {
            break;
        }
        case call_exp: {
            break;
        }
        case unop_exp: {
            break;
        }
        case var_exp: {
            break;
        }
        default:
            assert(0);
    }
}

void transformStmts(list * l, S_table globals_types, S_table function_rets, frame * f) {
    UNUSED(globals_types);
    UNUSED(function_rets);
    UNUSED(f);
    if (l == NULL) return;
    stmt_node * s = l->head;
    if(s) {
        switch(s->kind){
            case assign_stmt: {
                break;
            }
            case if_stmt: {
                exp_node * cond = s->data.if_ops.cond;
                list * then_stmts = s->data.if_ops.then_stmts;
                list * else_stmts = s->data.if_ops.else_stmts;
                transformExpr(cond, globals_types, function_rets, f);
                transformStmts(then_stmts, globals_types, function_rets, f);
                transformStmts(else_stmts, globals_types, function_rets, f);
                break;
            }
            case while_stmt: {
                exp_node * cond = s->data.while_ops.cond;
                list * while_body = s->data.while_ops.body;
                list * otherwise_body = s->data.while_ops.otherwise;
                transformExpr(cond, globals_types, function_rets, f);
                transformStmts(while_body, globals_types, function_rets, f);
                transformStmts(otherwise_body, globals_types, function_rets, f);

                if(otherwise_body != NULL) {
                    printf("transform: while-otherwise -> if-while-else\n");
                    s->kind = if_stmt;
                    s->data.if_ops.cond = cond;
                    s->data.if_ops.then_stmts = ListAddLast(WhileNode(cond, ListCopy(while_body), NULL),
                                                            while_body);
                    s->data.if_ops.else_stmts = otherwise_body;
                }
                break;
            }
            case repeat_stmt: {

                exp_node * cond = s->data.repeat_ops.cond;
                list * body = s->data.repeat_ops.body;
                transformStmts(body, globals_types, function_rets, f);

                char * counter_var = generateFreshGlobalIntVar();

                // Add the variable to global variables declarations
                list * var_decs = p.variables;
                vardec_node * counter_var_dec = VarDecNode(counter_var, IntTyNode(), IntNode(0));
                p.variables = ListAddLast(counter_var_dec, var_decs);
                S_enter(globals_types, S_Symbol(counter_var), IntTyNode());


                s->kind = assign_stmt;
                s->data.assign_ops.lhs = counter_var;
                s->data.assign_ops.rhs = cond;

                stmt_node * while_stmt = WhileNode(
                        BinOpNode(gt_op, VarOpNode(counter_var), IntNode(0)),
                        ListAddLast(
                                AssignNode(
                                        counter_var,
                                        BinOpNode(minus_op, VarOpNode(counter_var), IntNode(1))
                                ),
                                body
                        ),
                        NULL
                );

                list * next = l->next;
                list * while_node = ListAddLast(while_stmt, NULL);
                l->next = while_node;
                while_node->next = next;
                l = l->next;
                break;
            }
            case ret_stmt: {
                if(f == NULL) {
                    // We are in the top level, replace with intrinsic call exit
                    exp_node * ret = s->data.ret_exp;
                    s->kind = intrinsic_stmt;
                    s->data.intrinsic_ops.name = "exit";
                    s->data.intrinsic_ops.args = ListAddFirst(ret, NULL);
                } else {
                    transformExpr(s->data.ret_exp, globals_types, function_rets, f);
                }
                break;
            }
            case call_stmt: {
                if(!strcmp("exit", s->data.call_ops.name)){
                    char* name = s->data.call_ops.name;
                    list * args = s->data.call_ops.args;
                    s->kind = intrinsic_stmt;
                    s->data.intrinsic_ops.name = name;
                    s->data.intrinsic_ops.args = args;

                } else if(!strcmp("printint", s->data.call_ops.name)) {
                    printf("transform: call_stmt -> intrinsic:printint\n");
                    char* name = s->data.call_ops.name;
                    list * args = s->data.call_ops.args;
                    exp_node * arg = (exp_node*) args->head;
                    transformExpr(arg, globals_types, function_rets, f);
                    s->kind = intrinsic_stmt;
                    s->data.intrinsic_ops.name = name;
                    s->data.intrinsic_ops.args = args;
                } else if(!strcmp("printstring", s->data.call_ops.name)) {
                    printf("transform: call_stmt -> intrinsic:printstring\n");
                    char* name = s->data.call_ops.name;
                    list * args = s->data.call_ops.args;
                    exp_node * arg = (exp_node*) args->head;
                    transformExpr(arg, globals_types, function_rets, f);
                    s->kind = intrinsic_stmt;
                    s->data.intrinsic_ops.name = name;
                    s->data.intrinsic_ops.args = args;
                } else {
                    list * args = s->data.call_ops.args;
                    while(args != NULL) {
                        transformExpr((exp_node *) args->head, globals_types, function_rets, f);
                        args = args->next;
                    }
                }
                break;
            }
            default:
                assert(0);
        }
    }

    transformStmts(l->next, globals_types, function_rets, f);
}

void transformVariable(vardec_node * node, S_table globals_types, S_table function_rets, frame * f) {
    UNUSED(node); UNUSED(globals_types); UNUSED(function_rets); UNUSED(f);
//    transformExpr(node->init, globals_types, function_rets, f);
}

void transformVariables(list * l, S_table global_types, S_table function_rets, frame * f) {
    if (l == NULL) return;
    transformVariable((vardec_node *)l->head, global_types, function_rets, f);
    transformVariables(l->next, global_types, function_rets, f);
}

void transformFunction(fundec_node * fundec, S_table globals, S_table functions_rets, frame * f) {
    UNUSED(fundec);
    transformStmts(fundec->stmts, globals, functions_rets, f);
    stmt_node * ret = RetNode(NULL);
    ListAddLast(ret, fundec->stmts);

}

void transformFunctions(list * l, S_table global_types, S_table function_ret_types, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    frame * f = S_look(frames, S_Symbol(fundec->name));
    transformFunction(fundec, global_types, function_ret_types, f);
    transformFunctions(l->next, global_types, function_ret_types, frames);
}

void transform(program * p, S_table global_types, S_table function_rets, S_table frames) {

    transformVariables(p->variables, global_types, function_rets, NULL);
    transformFunctions(p->functions, global_types, function_rets, frames);

    p->statements = ListAddLast(
            RetNode(NULL),
            p->statements
    );
    transformStmts(p->statements, global_types, function_rets, NULL);
}
