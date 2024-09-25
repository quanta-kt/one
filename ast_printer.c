#include "ast_printer.h"

#include <stdio.h>

#include "ast.h"

AST_EXPR_WALKER(ast_expr_printer_t, void)

AST_STMT_WALKER(ast_stmt_printer_t, void)

static void walk_num(ast_expr_printer_t* _, ast_node_num* node) {
    printf("%Le", node->value);
}

static void walk_iden(ast_expr_printer_t* _, ast_node_identifier* node) {
    printf("%.*s", (int)node->len, node->start);
}

static void walk_binary(ast_expr_printer_t* self, ast_node_binary* node) {
    char op = '?';

    switch (node->op) {
        case TOK_DIV:
            op = '/';
            break;

        case TOK_MUL:
            op = '*';
            break;

        case TOK_PLUS:
            op = '+';
            break;

        case TOK_MINUS:
            op = '-';
            break;
    }

    printf("(%c ", op);
    ast_expr_printer_t_walk(self, node->left);
    printf(" ");
    ast_expr_printer_t_walk(self, node->right);
    printf(")");
}

static void walk_str(ast_expr_printer_t* _, ast_node_str* node) {
    printf("(str '%s')", node->str);
}

static void walk_bool(ast_expr_printer_t* _, ast_node_bool* node) {
    printf("%s", node->value ? "true" : "false");
}

ast_expr_printer_t expr_printer = (ast_expr_printer_t){
    .walk_binary = walk_binary,
    .walk_num = walk_num,
    .walk_iden = walk_iden,
    .walk_str = walk_str,
    .walk_bool = walk_bool,
};

static void print_expr(ast_expr_node* node) {
    ast_expr_printer_t_walk(&expr_printer, node);
}

static void walk_expr_stmt(ast_stmt_printer_t* self, ast_node_expr_stmt* node) {
    print_expr(node->expr);
    printf(";");
}

static void walk_var_decl(ast_stmt_printer_t* self, ast_node_var_decl* node) {
    printf("(let %.*s ", (int)node->name.span_size, node->name.span);
    if (node->value == NULL) {
        printf("NULL");
    } else {
        print_expr(node->value);
    }
    printf(");");
}

ast_stmt_printer_t stmt_printer = (ast_stmt_printer_t){
    .walk_expr_stmt = walk_expr_stmt,
    .walk_var_decl = walk_var_decl,
};

void print_ast(ast_stmt_node* node) {
    ast_stmt_node* curr = node;

    while (curr != NULL) {
        ast_stmt_printer_t_walk(&stmt_printer, curr);
        putchar('\n');
        curr = curr->next;
    }
}
