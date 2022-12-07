#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"

static ty_node int_ty_node    = { int_ty };
static ty_node string_ty_node = { string_ty };
static ty_node void_ty_node   = { void_ty };

ty_node* IntTyNode(){
    return &int_ty_node;
}

ty_node* StringTyNode(){
    return &string_ty_node;
}

ty_node* VoidTyNode(){
    return &void_ty_node;
}

ty_node* TyNode(char * type) {
    if (!strcmp(type,"int")) {
        return IntTyNode();
    } else if (!strcmp(type, "string")) {
        return StringTyNode();
    } else if (!strcmp(type, "void")) {
        return VoidTyNode();
    }
    assert(0);
}

char* typeToStr(ty_node* type) {
    switch (type->kind) {
        case int_ty:
            return "int";
        case string_ty:
            return "string";
        case void_ty:
            return "void";
        default:
            assert(0);
    }
}

param * Param(ty_node * type, char * name) {
    param * p = malloc(sizeof(param));
    p->name = name;
    p->ty = type;
    return p;
}

exp_node* IntNode(int val){
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = int_exp;
    node->data.ival = val;
    return node;
}

exp_node* StringNode(char * str) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = string_exp;
    node->data.sval = str;
    return node;
}

exp_node* BinOpNode(binop operation, exp_node * left, exp_node * right) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = binop_exp;
    node->data.bin_ops.op = operation;
    node->data.bin_ops.e1 = left;
    node->data.bin_ops.e2 = right;
    return node;
}

exp_node* UnOpNode(unop operation, exp_node * expr) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = unop_exp;
    node->data.un_ops.op = operation;
    node->data.un_ops.e = expr;
    return node;
}

exp_node* VarOpNode(char * name) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = var_exp;
    node->data.var_ops.name = name;
    return node;
}

exp_node* CallOpNode(char * name, list * args) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = call_exp;
    node->data.call_ops.name = name;
    node->data.call_ops.args = args;
    return node;
}

exp_node* IntrinsicNode(char * name, list * args) {
    exp_node* node = CallOpNode(name, args);
    node->kind = intrinsic_exp;
    return node;
}

stmt_node* CallOpStmtNode(char * name, list * args) {
    stmt_node * node = malloc(sizeof(stmt_node));
    node->kind = call_stmt;
    node->data.call_ops.name = name;
    node->data.call_ops.args = args;
    return node;
}

stmt_node* IntrinsicStmtNode(char * name, list * args) {
    stmt_node * node = CallOpStmtNode(name, args);
    node->kind = intrinsic_stmt;
    return node;
}

stmt_node* RetNode(exp_node* e){
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = ret_stmt;
    node->data.ret_exp = e;
    return node;
}

vardec_node * VarDecNode(char * name, ty_node * type, exp_node * init) {
    vardec_node * node = malloc(sizeof(vardec_node ));
    node->name = name;
    node->type = type;
    node->init = init;
    return node;
}

fundec_node * FunDecNode(char * name, ty_node * type, list * args, list * locs, list * stmts) {
    fundec_node * node = malloc(sizeof(fundec_node ));
    node->name = name;
    node->type = type;
    node->locs = locs;
    node->stmts = stmts;
    node->args = args;
    return node;
}

stmt_node* IfNode(exp_node* cond, list * thenStmts, list * elseStmts) {
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = if_stmt;
    node->data.if_ops.cond = cond;
    node->data.if_ops.then_stmts = thenStmts;
    node->data.if_ops.else_stmts = elseStmts;
    return node;
}

stmt_node* WhileNode(exp_node* guard, list * body, list * otherwise) {
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = while_stmt;
    node->data.while_ops.cond = guard;
    node->data.while_ops.body = body;
    node->data.while_ops.otherwise = otherwise;
    return node;
}

stmt_node* AssignNode(char * lhs, exp_node * rhs) {
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = assign_stmt;
    node->data.assign_ops.lhs = lhs;
    node->data.assign_ops.rhs = rhs;
    return node;
}

stmt_node* RepeatNode(exp_node* n, list * body) {
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = repeat_stmt;
    node->data.repeat_ops.cond = n;
    node->data.repeat_ops.body = body;
    return node;
}

list* ListAddFirst(void* hd, list* lst){
    list* l = malloc(sizeof(list));
    l->head = hd;
    l->next = lst;
    return l;
}

list* ListCopy(list* lst) {
    if (lst == NULL) {
        return NULL;
    }

    list * ret = malloc(sizeof(list));
    ret->head = lst->head;
    ret->next = ListCopy(lst->next);

    return ret;
}

list* ListAddLast(void* hd, list* lst){
    list* l = malloc(sizeof(list));
    l->head = hd;
    l->next = NULL;

    // Empty list
    if (lst == NULL) {
        return l;
    }

    // Find last
    list * last = lst;
    while (last->next != NULL) {
        last = last->next;
    }


    last->next = l;
    return lst;
}
