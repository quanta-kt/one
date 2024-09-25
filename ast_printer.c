#include "ast_printer.h"

#include <stdio.h>

#include "ast.h"

AST_EXPR_WALKER(ast_printer_t, void)

static void walk_num(ast_printer_t* _, ast_node_num* node) {
    printf("%Le", node->value);
}

static void walk_iden(ast_printer_t* _, ast_node_identifier* node) {
    printf("%.*s", (int)node->len, node->start);
}

static void walk_binary(ast_printer_t* self, ast_node_binary* node) {
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
    ast_printer_t_walk(self, node->left);
    printf(" ");
    ast_printer_t_walk(self, node->right);
    printf(")");
}

static void walk_str(ast_printer_t* _, ast_node_str* node) {
    printf("(str '%s')", node->str);
}

static void walk_bool(ast_printer_t* _, ast_node_bool* node) {
    printf("%s", node->value ? "true" : "false");
}

ast_printer_t walk_printer = (ast_printer_t){
    .walk_binary = walk_binary,
    .walk_num = walk_num,
    .walk_iden = walk_iden,
    .walk_str = walk_str,
    .walk_bool = walk_bool,
};

void print_ast(ast_expr_node* node) {
    ast_printer_t_walk(&walk_printer, node);
    putchar('\n');
}
