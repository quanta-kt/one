#include "ast_printer.h"

#include <stdio.h>

#include "ast.h"

AST_EXPR_WALKER(ast_expr_printer_t, void)

AST_STMT_WALKER(ast_stmt_printer_t, void)

AST_ITEM_WALKER(ast_item_printer_t, void)

static void walk_num(ast_expr_printer_t* _, ast_node_num* node) {
    printf("%Le", node->value);
}

static void walk_iden(ast_expr_printer_t* _, ast_node_identifier* node) {
    printf("%.*s", (int)node->len, node->start);
}

static char* op2str(token_type tt) {
    char* op = "?";

    switch (tt) {
        case TOK_DIV:
            op = "/";
            break;

        case TOK_MUL:
            op = "*";
            break;

        case TOK_PERC:
            op = "%";
            break;

        case TOK_PLUS:
            op = "+";
            break;

        case TOK_MINUS:
            op = "-";
            break;

        case TOK_PIPE:
            op = "|";
            break;

        case TOK_AMP:
            op = "&";
            break;

        case TOK_CARET:
            op = "^";
            break;

        case TOK_OR:
            op = "||";
            break;

        case TOK_AND:
            op = "&&";
            break;

        case TOK_BANG:
            op = "!";
            break;

        case TOK_EQ:
            op = "==";
            break;

        case TOK_NEQ:
            op = "!=";
            break;

        case TOK_GT:
            op = ">";
            break;

        case TOK_GTEQ:
            op = ">=";
            break;

        case TOK_LT:
            op = "<";
            break;

        case TOK_LTEQ:
            op = "<=";
            break;
    }

    return op;
}

static void walk_binary(ast_expr_printer_t* self, ast_node_binary* node) {
    char* op = op2str(node->op);

    printf("(%s ", op);
    ast_expr_printer_t_walk(self, node->left);
    printf(" ");
    ast_expr_printer_t_walk(self, node->right);
    printf(")");
}

static void walk_unary(ast_expr_printer_t* self, ast_node_unary* node) {
    char* op = op2str(node->op);
    printf("(%s ", op);
    ast_expr_printer_t_walk(self, node->expr);
    putchar(')');
}

static void walk_call(ast_expr_printer_t* self, ast_node_call* node) {
    printf("(call ");
    ast_expr_printer_t_walk(self, node->function);

    ast_expr_node** arg;
    foreach_vec_item(&node->args, ast_expr_node*, arg) {
        putchar(' ');
        ast_expr_printer_t_walk(self, *arg);
    }

    putchar(')');
}

static void walk_str(ast_expr_printer_t* _, ast_node_str* node) {
    printf("(str '%s')", node->str);
}

static void walk_bool(ast_expr_printer_t* _, ast_node_bool* node) {
    printf("%s", node->value ? "true" : "false");
}

ast_expr_printer_t expr_printer = (ast_expr_printer_t){
    .walk_binary = walk_binary,
    .walk_unary = walk_unary,
    .walk_call = walk_call,
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
}

static void walk_var_decl(ast_stmt_printer_t* self, ast_node_var_decl* node) {
    char* op = node->mut ? "let-mut" : "let";

    printf("(%s %.*s ", op, (int)node->name.span_size, node->name.span);
    if (node->value == NULL) {
        printf("NULL");
    } else {
        print_expr(node->value);
    }
    printf(")");
}

static void walk_block(ast_stmt_printer_t* self, ast_node_block* node) {
    printf("(block");

    ast_stmt_node* curr = node->body;
    while (curr != NULL) {
        putchar(' ');
        ast_stmt_printer_t_walk(self, curr);
        curr = curr->next;
    }

    printf(")");
}

static void walk_if_else(ast_stmt_printer_t* self, ast_node_if_else* node) {
    printf("(if ");
    print_expr(node->condition);

    putchar(' ');
    ast_stmt_printer_t_walk(self, node->body);

    if (node->else_body != NULL) {
        putchar(' ');
        ast_stmt_printer_t_walk(self, node->else_body);
    }

    printf(")");
}

static void walk_while(ast_stmt_printer_t* self, ast_node_while* node) {
    printf("(while ");
    print_expr(node->condition);

    putchar(' ');
    ast_stmt_printer_t_walk(self, node->body);

    putchar(')');
}

ast_stmt_printer_t stmt_printer = (ast_stmt_printer_t){
    .walk_expr_stmt = walk_expr_stmt,
    .walk_var_decl = walk_var_decl,
    .walk_block = walk_block,
    .walk_if_else = walk_if_else,
    .walk_while = walk_while,
};

static void print_stmt(ast_stmt_node* node) {
    ast_stmt_node* curr = node;

    while (curr != NULL) {
        ast_stmt_printer_t_walk(&stmt_printer, curr);
        curr = curr->next;

        // If there is another statement on the list, we want a separator
        // between that and the current one.
        if (curr != NULL) {
            putchar(' ');
        }
    }
}

static void walk_function(ast_item_printer_t* self, ast_node_function* fn) {
    printf("(fn ");

    printf("%.*s ", (int)fn->name.span_size, fn->name.span);

    putchar('(');
    ast_param* curr = fn->params;
    while (curr != NULL) {
        printf("%.*s", (int)curr->name.span_size, curr->name.span);

        curr = curr->next;
        if (curr != NULL) {
            putchar(' ');
        }
    }
    putchar(')');

    if (fn->body != NULL) putchar(' ');
    print_stmt(fn->body);

    printf(")\n");
}

ast_item_printer_t item_printer =
    (ast_item_printer_t){.walk_function = walk_function};

void print_ast(ast_item_node* node) {
    ast_item_node* curr = node;

    while (curr != NULL) {
        ast_item_printer_t_walk(&item_printer, curr);
        curr = curr->next;
    }
}
