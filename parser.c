// TODO: Handle unexpected tokens in the parser

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

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

Tree_Node *make_node_binop(Arena *a, Node_Type t, Tree_Node *lhs, Tree_Node *rhs)
{
    Tree_Node *r = arena_alloc(a, sizeof(*r));
    r->type = t;
    r->binop.lhs = lhs;
    r->binop.rhs = rhs;
    return r;
}

Tree_Node *make_node_unary(Arena *a, Node_Type t, Tree_Node *node)
{
    Tree_Node *r = arena_alloc(a, sizeof(*r));
    r->type = t;
    r->unary.node = node;
    return r;
}

Tree_Node *make_node(Arena *a, Node_Type t, double value)
{
    Tree_Node *r = arena_alloc(a, sizeof(*r));
    r->type = t;
    r->value = value;
    return r;
}

Result parse(Arena *a, Parse_Tree *tree, Token_List list)
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

    Tree_Node *tree_root = parse_expr(a, &parser, &result);
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

Tree_Node *parse_expr(Arena *a, Parser *parser, Result *result)
{
    Tree_Node *result_node = parse_term(a, parser, result);
    Token *cur = &parser->current;

    while (!result->error
        && (cur->type == TOKEN_PLUS || cur->type == TOKEN_MINUS)) {

        if (cur->type == TOKEN_PLUS) {
            parser_advance(parser);
            result_node = make_node_binop(a, NODE_ADD, result_node,
                                          parse_term(a, parser, result));
        } else if (cur->type == TOKEN_MINUS) {
            parser_advance(parser);
            result_node = make_node_binop(a, NODE_SUBTRACT, result_node,
                                          parse_term(a, parser, result));
        }
    }

    return result_node;
}

Tree_Node *parse_term(Arena *a, Parser *parser, Result *result)
{
    Tree_Node *result_node = parse_factor(a, parser, result);
    Token *cur = &parser->current;

    while (!result->error
        && (cur->type == TOKEN_MULTIPLY || cur->type == TOKEN_DIVIDE)) {

        if (cur->type == TOKEN_MULTIPLY) {
            parser_advance(parser);
            result_node = make_node_binop(a, NODE_MULTIPLY, result_node,
                                          parse_factor(a, parser, result));
        } else if (cur->type == TOKEN_DIVIDE) {
            result_node = make_node_binop(a, NODE_DIVIDE, result_node,
                                          parse_factor(a, parser, result));
        }
    }

    return result_node;
}

Tree_Node *parse_factor(Arena *a, Parser *parser, Result *result)
{
    Token *cur = &parser->current;

    if (cur->type == TOKEN_LPAREN) {
        parser_advance(parser);
        Tree_Node *result_node = parse_expr(a, parser, result);

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
        Tree_Node *number = make_node(a, NODE_NUMBER, cur->value);
        parser_advance(parser);
        return number;
    }

    if (cur->type == TOKEN_PLUS) {
        parser_advance(parser);
        return make_node_unary(a, NODE_PLUS, parse_factor(a, parser, result));
    }

    if (cur->type == TOKEN_MINUS) {
        parser_advance(parser);
        return make_node_unary(a, NODE_MINUS, parse_factor(a, parser, result));
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
