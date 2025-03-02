#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>
#include <stddef.h>

#include "arena.h"

typedef enum {
    TOKEN_INVALID,
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COUNT
} Token_Type;

typedef struct {
    Token_Type type;
    size_t position;
    double value;
} Token;

typedef struct {
    size_t count;
    size_t capacity;
    Token *items;
} Token_List;

typedef enum {
    ERROR_OK,
    ERROR_INVALID_TOKEN,
    ERROR_INVALID_EXPRESSION,
    ERROR_EMPTY_EXPRESSION,
    ERROR_INVALID_NODE,
    ERROR_ZERO_DIVISION,
    ERROR_COUNT
} Error_Type;

typedef struct {
    bool error;
    Error_Type error_type;
    size_t error_position;
    Token faulty_token;
    Token_Type expected_token;
    double value;
} Result;

const char *error_to_string(Error_Type err);

typedef enum {
    NODE_INVALID,
    NODE_NUMBER,
    NODE_ADD,
    NODE_SUBTRACT,
    NODE_MULTIPLY,
    NODE_DIVIDE,
    NODE_PLUS,
    NODE_MINUS,
    NODE_COUNT
} Node_Type;

typedef struct Tree_Node Tree_Node;

struct Tree_Node {
    Node_Type type;

    union {
        struct {
            Tree_Node *lhs;
            Tree_Node *rhs;
        } binop;

        struct {
            Tree_Node *node;
        } unary;

        double value;
    };
};

typedef struct {
    Token_List tokens;
    Token current;
    size_t cursor;
} Parser;

typedef struct {
    Tree_Node *root;
    Result result;
} Parse_Tree;

Result tokenize(Token_List *list, const char *expr);
const char *token_to_string(Token token);
void print_token_list(Token_List list);

const char *error_to_string(Error_Type err);

Tree_Node *make_node_binop(Arena *a, Node_Type t, Tree_Node *lhs, Tree_Node *rhs);
Tree_Node *make_node_unary(Arena *a, Node_Type t, Tree_Node *node);
Tree_Node *make_node(Arena *a, Node_Type t, double value);

Result parse(Arena *a, Parse_Tree *tree, Token_List list);
void parser_advance(Parser *parser);
Tree_Node *parse_expr(Arena *a, Parser *parser, Result *result);
Tree_Node *parse_term(Arena *a, Parser *parser, Result *result);
Tree_Node *parse_factor(Arena *a, Parser *parser, Result *result);
void print_parse_tree(Parse_Tree tree);
void print_tree_node(Tree_Node *root);

#endif // PARSER_H_
