// TODO: Handle unexpected tokens in the parser

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define UNREACHABLE() do { assert(0 && "UNREACHABLE"); } while (0)

#define da_append(da, item)                                                            \
    do {                                                                               \
        if ((da)->count >= (da)->capacity) {                                           \
            (da)->capacity = (da)->capacity == 0 ? 128 : (da)->capacity * 2;           \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Out of memory");                            \
        }                                                                              \
        (da)->items[(da)->count++] = (item);                                           \
    } while (0)

#define da_free(da)         \
    do {                    \
        free((da)->items);  \
        (da)->items = NULL; \
        (da)->count = 0;    \
        (da)->capacity = 0; \
    } while (0)

#define da_reset(da)     \
    do {                 \
        (da)->count = 0; \
    } while (0)

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

const char *error_to_string(Error_Type err);

Result tokenize(Token_List *list, const char *expr);
const char *token_to_string(Token token);
void print_token_list(Token_List list);

Result parse(Arena *arena, Parse_Tree *tree, Token_List list);
void parser_advance(Parser *parser);
Tree_Node *parse_expr(Arena *arena, Parser *parser, Result *result);
Tree_Node *parse_term(Arena *arena, Parser *parser, Result *result);
Tree_Node *parse_factor(Arena *arena, Parser *parser, Result *result);
void print_parse_tree(Parse_Tree tree);
void print_tree_node(Tree_Node *root);

Result interpret(Parse_Tree tree);
Result interpret_node(Tree_Node *root);

const char *error_to_string(Error_Type err)
{
    switch (err) {
        case ERROR_OK:                 return "No error";
        case ERROR_INVALID_TOKEN:      return "Unexpected token";
        case ERROR_INVALID_EXPRESSION: return "Invalid expression";
        case ERROR_EMPTY_EXPRESSION:   return "Empty expression";
        case ERROR_INVALID_NODE:       return "Invalid expression";
        case ERROR_ZERO_DIVISION:      return "Division by zero";
        default:                       return "Unknown error";
    }
}

Result tokenize(Token_List *list, const char *expr)
{
    Result result = {0};
    size_t cursor = 0;
    size_t end = strlen(expr);

    while (cursor < end) {
        char c = expr[cursor];
        Token token = {0};
        token.position = cursor;

        switch (c) {
            case '\n':
            case '\t':
            case ' ': {
                ++cursor;
            } break;

            case '+': {
                token.type = TOKEN_PLUS;
                da_append(list, token);
                ++cursor;
            } break;

            case '-': {
                token.type = TOKEN_MINUS;
                da_append(list, token);
                ++cursor;
            } break;

            case '*': {
                token.type = TOKEN_MULTIPLY;
                da_append(list, token);
                ++cursor;
            } break;

            case '/': {
                token.type = TOKEN_DIVIDE;
                da_append(list, token);
                ++cursor;
            } break;

            case '(': {
                token.type = TOKEN_LPAREN;
                da_append(list, token);
                ++cursor;
            } break;

            case ')': {
                token.type = TOKEN_RPAREN;
                da_append(list, token);
                ++cursor;
            } break;

            default: {
                if (isdigit(c)) {
                    char *end;
                    token.type = TOKEN_NUMBER;
                    token.value = strtod(&expr[cursor], &end);
                    cursor = end - expr;
                    da_append(list, token);
                    break;
                }

                token.type = TOKEN_INVALID;

                result.error = true;
                result.error_type = ERROR_INVALID_TOKEN;
                result.error_position = cursor;
                result.faulty_token = token;
                return result;

            } break;
        }
    }

    return result;
}

const char *token_to_string(Token token)
{
    switch (token.type) {
        case TOKEN_EOF:      return "TOKEN_EOF";
        case TOKEN_INVALID:  return "TOKEN_INVALID";
        case TOKEN_NUMBER:   return "TOKEN_NUMBER";
        case TOKEN_PLUS:     return "TOKEN_PLUS";
        case TOKEN_MINUS:    return "TOKEN_MINUS";
        case TOKEN_MULTIPLY: return "TOKEN_MULTIPLY";
        case TOKEN_DIVIDE:   return "TOKEN_DIVIDE";
        case TOKEN_LPAREN:   return "TOKEN_LPAREN";
        case TOKEN_RPAREN:   return "TOKEN_RPAREN";
        default: {
            UNREACHABLE();
        } break;
    }
}

void print_token_list(Token_List list)
{
    for (size_t i = 0; i < list.count; ++i) {
        Token token = list.items[i];
        printf("Token %ld: type %s", i, token_to_string(token));
        if (token.type == TOKEN_NUMBER) {
            printf(", value %f", token.value);
        }
        printf("\n");
    }
}

Result parse(Arena *arena, Parse_Tree *tree, Token_List list)
{
    Result result = {0};

    Parser parser = {0};
    parser.tokens = list;

    parser_advance(&parser);

    if (parser.current.type == TOKEN_EOF) {
        result.error = true;
        result.error_type = ERROR_EMPTY_EXPRESSION;
        return result;
    }

    Tree_Node *tree_root = parse_expr(arena, &parser, &result);
    tree->root = tree_root;

    if (parser.current.type != TOKEN_EOF) {
        result.error = true;
        result.error_type = ERROR_INVALID_EXPRESSION;
        result.error_position = parser.current.position;
        return result;
    }

    return result;
}

void parser_advance(Parser *parser)
{
    if (parser->cursor >= parser->tokens.count) {
        parser->current.type = TOKEN_EOF;
        return;
    }

    parser->current = parser->tokens.items[parser->cursor++];
}

Tree_Node *parse_expr(Arena *arena, Parser *parser, Result *result)
{
    Tree_Node *result_node = parse_term(arena, parser, result);
    Token *cur = &parser->current;

    while (!result->error
        && (cur->type == TOKEN_PLUS || cur->type == TOKEN_MINUS)) {

        if (cur->type == TOKEN_PLUS) {
            parser_advance(parser);
            Tree_Node *add = arena_alloc(arena, sizeof(*add));
            add->type = NODE_ADD;
            add->binop.lhs = result_node;
            add->binop.rhs = parse_term(arena, parser, result);
            result_node = add;
        } else if (cur->type == TOKEN_MINUS) {
            parser_advance(parser);
            Tree_Node *sub = arena_alloc(arena, sizeof(*sub));
            sub->type = NODE_SUBTRACT;
            sub->binop.lhs = result_node;
            sub->binop.rhs = parse_term(arena, parser, result);
            result_node = sub;
        }
    }

    return result_node;
}

Tree_Node *parse_term(Arena *arena, Parser *parser, Result *result)
{
    Tree_Node *result_node = parse_factor(arena, parser, result);
    Token *cur = &parser->current;

    while (!result->error
        && (cur->type == TOKEN_MULTIPLY || cur->type == TOKEN_DIVIDE)) {

        if (cur->type == TOKEN_MULTIPLY) {
            parser_advance(parser);
            Tree_Node *mul = arena_alloc(arena, sizeof(*mul));
            mul->type = NODE_MULTIPLY;
            mul->binop.lhs = result_node;
            mul->binop.rhs = parse_factor(arena, parser, result);
            result_node = mul;
        } else if (cur->type == TOKEN_DIVIDE) {
            parser_advance(parser);
            Tree_Node *div = arena_alloc(arena, sizeof(*div));
            div->type = NODE_DIVIDE;
            div->binop.lhs = result_node;
            div->binop.rhs = parse_factor(arena, parser, result);
            result_node = div;
        }
    }

    return result_node;
}

Tree_Node *parse_factor(Arena *arena, Parser *parser, Result *result)
{
    Token *cur = &parser->current;

    if (cur->type == TOKEN_LPAREN) {
        parser_advance(parser);
        Tree_Node *result_node = parse_expr(arena, parser, result);

        if (cur->type != TOKEN_RPAREN) {
            result->error = true;
            result->error_type = ERROR_INVALID_EXPRESSION;
            result->error_position = cur->position;
            result->expected_token = TOKEN_RPAREN;
            return result_node;
        }

        parser_advance(parser);
        return result_node;
    }

    if (cur->type == TOKEN_NUMBER) {
        Tree_Node *number = arena_alloc(arena, sizeof(*number));
        number->type = NODE_NUMBER;
        number->value = cur->value;
        parser_advance(parser);
        return number;
    }

    if (cur->type == TOKEN_PLUS) {
        parser_advance(parser);
        Tree_Node *plus = arena_alloc(arena, sizeof(*plus));
        plus->type = NODE_PLUS;
        plus->unary.node = parse_factor(arena, parser, result);
        return plus;
    }

    if (cur->type == TOKEN_MINUS) {
        parser_advance(parser);
        Tree_Node *minus = arena_alloc(arena, sizeof(*minus));
        minus->type = NODE_MINUS;
        minus->unary.node = parse_factor(arena, parser, result);
        return minus;
    }

    result->error = true;
    result->error_type = ERROR_INVALID_EXPRESSION;
    result->error_position = cur->position;
    result->faulty_token = *cur;

    return NULL;
}

void print_parse_tree(Parse_Tree tree)
{
    print_tree_node(tree.root);
    printf("\n");
}

void print_tree_node(Tree_Node *root)
{
    if (root == NULL)
        return;

    switch (root->type) {
        case NODE_INVALID: {
            printf("INVALID");
        } break;

        case NODE_NUMBER: {
            printf("%f", root->value);
        } break;

        case NODE_ADD: {
            printf("add(");
            print_tree_node(root->binop.lhs);
            printf(",");
            print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case NODE_SUBTRACT: {
            printf("sub(");
            print_tree_node(root->binop.lhs);
            printf(",");
            print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case NODE_MULTIPLY: {
            printf("mul(");
            print_tree_node(root->binop.lhs);
            printf(",");
            print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case NODE_DIVIDE: {
            printf("div(");
            print_tree_node(root->binop.lhs);
            printf(",");
            print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case NODE_PLUS: {
            printf("plus(");
            print_tree_node(root->unary.node);
            printf(")");
        } break;

        case NODE_MINUS: {
            printf("minus(");
            print_tree_node(root->unary.node);
            printf(")");
        } break;

        default: {
            printf("?");
        } break;
    }
}

Result interpret(Parse_Tree tree)
{
    if (tree.root == NULL) {
        Result r = {0};
        r.error = true;
        r.error_type = ERROR_EMPTY_EXPRESSION;
        return r;
    }

    return interpret_node(tree.root);
}

Result interpret_node(Tree_Node *root)
{
    Result result = {0};

    if (root == NULL) {
        result.error = true;
        result.error_type = ERROR_INVALID_NODE;
        return result;
    }

    switch (root->type) {
        case NODE_INVALID: {
            result.error = true;
            result.error_type = ERROR_INVALID_NODE;
            return result;
        } break;

        case NODE_NUMBER: {
            result.value = root->value;
        } break;

        case NODE_ADD: {
            Result a = interpret_node(root->binop.lhs);
            if (a.error) return a;
            Result b = interpret_node(root->binop.rhs);
            if (b.error) return b;
            result.value = a.value + b.value;
        } break;

        case NODE_SUBTRACT: {
            Result a = interpret_node(root->binop.lhs);
            if (a.error) return a;
            Result b = interpret_node(root->binop.rhs);
            if (b.error) return b;
            result.value = a.value - b.value;
        } break;

        case NODE_MULTIPLY: {
            Result a = interpret_node(root->binop.lhs);
            if (a.error) return a;
            Result b = interpret_node(root->binop.rhs);
            if (b.error) return b;
            result.value = a.value * b.value;
        } break;

        case NODE_DIVIDE: {
            Result b = interpret_node(root->binop.rhs);
            if (b.error) return b;
            if (b.value == 0.0) {
                result.error = true;
                result.error_type = ERROR_ZERO_DIVISION;
                return result;
            }

            Result a = interpret_node(root->binop.lhs);
            if (a.error) return a;
            result.value = a.value / b.value;
        } break;

        case NODE_PLUS: {
            result = interpret_node(root->unary.node);
        } break;

        case NODE_MINUS: {
            Result n = interpret_node(root->unary.node);
            result.value = -n.value;
        } break;

        default: {
            UNREACHABLE();
        } break;
    }

    return result;
}

int main(void)
{
    const char *expression = "(6 * 4) - 23 + (-3) * ((2 * -3) * 0.3) / 2";

    Arena arena = {0};
    Token_List token_list = {0};
    Parse_Tree parse_tree = {0};

    Result tr = tokenize(&token_list, expression);
    if (tr.error) {
        printf("%s\n", expression);
        printf("%*s^\n", (int)tr.error_position, "");
        printf("%ld: ERROR: %s\n", tr.error_position + 1,
               error_to_string(tr.error_type));
        goto cleanup;
    }

    Result pr = parse(&arena, &parse_tree, token_list);
    if (pr.error) {
        printf("%s\n", expression);
        printf("%*s^\n", (int)pr.error_position, "");
        printf("%ld: ERROR: %s\n", pr.error_position + 1,
               error_to_string(pr.error_type));
        goto cleanup;
    }

    Result r = interpret(parse_tree);
    if (r.error) {
        printf("ERROR: %s\n", error_to_string(r.error_type));
        goto cleanup;
    }

    printf("%s = %f\n", expression, r.value);

cleanup:
    da_free(&token_list);
    arena_free(&arena);

    return EXIT_SUCCESS;
}
