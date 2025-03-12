// mp - v1.1.0 - MIT License - https://github.com/seajee/mp.h

//----------------
// Header section
//----------------

#ifndef MP_H_
#define MP_H_

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//----------------
// Dynamic array
//----------------

#define MP_DA_INITIAL_CAPACITY 256

#define mp_da_append(da, item)                                               \
    do {                                                                     \
        if ((da)->count >= (da)->capacity) {                                 \
            (da)->capacity = (da)->capacity == 0                             \
                ? MP_DA_INITIAL_CAPACITY : (da)->capacity * 2;               \
            (da)->items =                                                    \
                realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM LOL");               \
        }                                                                    \
        (da)->items[(da)->count++] = (item);                                 \
    } while (0)

#define mp_da_free(da)      \
    do {                    \
        free((da)->items);  \
        (da)->items = NULL; \
        (da)->count = 0;    \
        (da)->capacity = 0; \
    } while (0)

#define mp_da_reset(da)  \
    do {                 \
        (da)->count = 0; \
    } while (0)

//-------
// Arena
//-------

// TODO: Implement the arena as a linked list of regions

#define MP_ARENA_DEFAULT_CAPACITY (8*1024)

typedef struct {
    size_t count;
    size_t capacity;
    void *data;
} MP_Arena;

MP_Arena mp_arena_init(size_t capacity);
void *mp_arena_alloc(MP_Arena *arena, size_t size);
void mp_arena_free(MP_Arena *arena);
void mp_arena_reset(MP_Arena *arena);

//-----------
// Tokenizer
//-----------

typedef enum {
    MP_TOKEN_INVALID,
    MP_TOKEN_EOF,
    MP_TOKEN_NUMBER,
    MP_TOKEN_SYMBOL,
    MP_TOKEN_PLUS,
    MP_TOKEN_MINUS,
    MP_TOKEN_MULTIPLY,
    MP_TOKEN_DIVIDE,
    MP_TOKEN_POWER,
    MP_TOKEN_LPAREN,
    MP_TOKEN_RPAREN,
    MP_TOKEN_COUNT
} MP_Token_Type;

typedef struct {
    MP_Token_Type type;
    size_t position;
    union {
        double value;
        char symbol;
    };
} MP_Token;

typedef struct {
    size_t count;
    size_t capacity;
    MP_Token *items;
} MP_Token_List;

//----------------
// Error handling
//----------------

// TODO: Simplify error handling

typedef enum {
    MP_ERROR_OK,
    MP_ERROR_INVALID_TOKEN,
    MP_ERROR_INVALID_EXPRESSION,
    MP_ERROR_EMPTY_EXPRESSION,
    MP_ERROR_INVALID_NODE,
    MP_ERROR_ZERO_DIVISION,
    MP_ERROR_COUNT
} MP_Error_Type;

typedef struct {
    bool error;
    MP_Error_Type error_type;
    size_t error_position;
    MP_Token faulty_token;
    double value;
} MP_Result;

const char *mp_error_to_string(MP_Error_Type err);

//---------------------
// Tokenizer functions
//---------------------

MP_Result mp_tokenize(MP_Token_List *list, const char *expr);
const char *mp_token_to_string(MP_Token token);
void mp_print_token_list(MP_Token_List list);

//--------
// Parser
//--------

typedef enum {
    MP_NODE_INVALID,
    MP_NODE_NUMBER,
    MP_NODE_SYMBOL,
    MP_NODE_ADD,
    MP_NODE_SUBTRACT,
    MP_NODE_MULTIPLY,
    MP_NODE_DIVIDE,
    MP_NODE_POWER,
    MP_NODE_PLUS,
    MP_NODE_MINUS,
    MP_NODE_COUNT
} MP_Node_Type;

typedef struct MP_Tree_Node MP_Tree_Node;

struct MP_Tree_Node {
    MP_Node_Type type;

    union {
        struct {
            MP_Tree_Node *lhs;
            MP_Tree_Node *rhs;
        } binop;

        struct {
            MP_Tree_Node *node;
        } unary;

        union {
            double value;
            char symbol;
        };
    };
};

typedef struct {
    MP_Token_List tokens;
    MP_Token current;
    size_t cursor;
} MP_Parser;

typedef struct {
    MP_Tree_Node *root;
    MP_Result result;
} MP_Parse_Tree;

MP_Tree_Node *mp_make_node(MP_Arena *a, MP_Node_Type t, double value);
MP_Tree_Node *mp_make_node_symbol(MP_Arena *a, char symbol);
MP_Tree_Node *mp_make_node_unary(MP_Arena *a, MP_Node_Type t, MP_Tree_Node *node);
MP_Tree_Node *mp_make_node_binop(MP_Arena *a, MP_Node_Type t,
                                 MP_Tree_Node *lhs, MP_Tree_Node *rhs);

MP_Result mp_parse(MP_Arena *a, MP_Parse_Tree *tree, MP_Token_List list);
void mp_parser_advance(MP_Parser *parser);
MP_Tree_Node *mp_parse_expr(MP_Arena *a, MP_Parser *parser, MP_Result *result);
MP_Tree_Node *mp_parse_term(MP_Arena *a, MP_Parser *parser, MP_Result *result);
MP_Tree_Node *mp_parse_factor(MP_Arena *a, MP_Parser *parser, MP_Result *result);
void mp_print_parse_tree(MP_Parse_Tree tree);
void mp_print_tree_node(MP_Tree_Node *root);

//-------------
// Interpreter
//-------------

// TODO: Interpreter does not support variables

MP_Result mp_interpret(MP_Parse_Tree tree);
MP_Result mp_interpret_node(MP_Tree_Node *root);

//----------
// Compiler
//----------

typedef enum {
    MP_OP_INVALID,
    MP_OP_PUSH_NUM,
    MP_OP_PUSH_VAR,
    MP_OP_ADD,
    MP_OP_SUB,
    MP_OP_MUL,
    MP_OP_DIV,
    MP_OP_POW,
    MP_OP_NEG,
    MP_OP_COUNT
} MP_Opcode;

typedef struct {
    size_t count;
    size_t capacity;
    uint8_t *items;
} MP_Program;

typedef struct {
    size_t count;
    size_t capacity;
    double *items;
} MP_Stack;

typedef struct {
    MP_Program program;
    MP_Stack stack;
    double vars[26]; // a - z
    size_t ip;
} MP_Vm;

typedef struct {
    bool present;
    double value;
} MP_Optional;

bool mp_program_compile(MP_Program *p, MP_Parse_Tree parse_tree);
bool mp_program_compile_node(MP_Program *p, MP_Tree_Node *node);
void mp_program_push_opcode(MP_Program *p, MP_Opcode op);
void mp_program_push_const(MP_Program *p, double value);
void mp_program_push_var(MP_Program *p, char var);
void mp_print_program(MP_Program p);

void mp_stack_push(MP_Stack *stack, double n);
MP_Optional mp_stack_pop(MP_Stack *stack);
MP_Optional mp_stack_peek(MP_Stack *stack);

MP_Vm mp_vm_init(MP_Program program);
void mp_vm_var(MP_Vm *vm, char var, double value);
bool mp_vm_run(MP_Vm *vm);
double mp_vm_result(MP_Vm *vm);
void mp_vm_free(MP_Vm *vm);

//-----------------------------------
// Simplified API (uses compilation)
//-----------------------------------

typedef struct {
    MP_Program program;
    MP_Vm vm;
} MP_Env;

MP_Env *mp_init(const char *expression);
void mp_variable(MP_Env *env, char var, double value);
MP_Result mp_evaluate(MP_Env *env);
void mp_free(MP_Env *env);

#endif // MP_H_

//------------------------
// Implementation section
//------------------------

#ifdef MP_IMPLEMENTATION

//-------
// Arena
//-------

MP_Arena mp_arena_init(size_t capacity)
{
    MP_Arena arena = {0};
    arena.capacity = capacity;
    arena.count = 0;
    arena.data = malloc(capacity);
    assert(arena.data != NULL);

    return arena;
}

void *mp_arena_alloc(MP_Arena *arena, size_t size)
{
    if (arena->data == NULL) {
        *arena = mp_arena_init(MP_ARENA_DEFAULT_CAPACITY);
    }

    assert(arena->count + size <= arena->capacity);

    void *result = (uint8_t*)arena->data + arena->count;
    arena->count += size;

    return result;
}

void mp_arena_free(MP_Arena *arena)
{
    arena->count = 0;
    arena->capacity = 0;
    free(arena->data);
}

void mp_arena_reset(MP_Arena *arena)
{
    arena->count = 0;
}

//-----------
// Tokenizer
//-----------

MP_Result mp_tokenize(MP_Token_List *list, const char *expr)
{
    MP_Result result = {0};
    size_t cursor = 0;
    size_t end = strlen(expr);

    while (cursor < end) {
        char c = expr[cursor];
        MP_Token token = {0};
        token.position = cursor;

        switch (c) {
            case '\n':
            case '\t':
            case ' ': {
                ++cursor;
            } break;

            case '+': {
                token.type = MP_TOKEN_PLUS;
                mp_da_append(list, token);
                ++cursor;
            } break;

            case '-': {
                token.type = MP_TOKEN_MINUS;
                mp_da_append(list, token);
                ++cursor;
            } break;

            case '*': {
                token.type = MP_TOKEN_MULTIPLY;
                mp_da_append(list, token);
                ++cursor;
            } break;

            case '/': {
                token.type = MP_TOKEN_DIVIDE;
                mp_da_append(list, token);
                ++cursor;
            } break;

            case '^': {
                token.type = MP_TOKEN_POWER;
                mp_da_append(list, token);
                ++cursor;
            } break;

            case '(': {
                token.type = MP_TOKEN_LPAREN;
                mp_da_append(list, token);
                ++cursor;
            } break;

            case ')': {
                token.type = MP_TOKEN_RPAREN;
                mp_da_append(list, token);
                ++cursor;
            } break;

            default: {
                // Numbers
                if (isdigit(c)) {
                    char *end;
                    token.type = MP_TOKEN_NUMBER;
                    token.value = strtod(&expr[cursor], &end);
                    cursor = end - expr;
                    mp_da_append(list, token);
                    break;
                }

                // Symbols
                if (islower(c)) {
                    if (cursor >= end - 1 || !islower(expr[cursor + 1])) {
                        token.type = MP_TOKEN_SYMBOL;
                        token.symbol = c;
                        mp_da_append(list, token);
                        ++cursor;
                        break;
                    }
                }

                // Invalid
                token.type = MP_TOKEN_INVALID;
                result.error = true;
                result.error_type = MP_ERROR_INVALID_TOKEN;
                result.error_position = cursor;
                result.faulty_token = token;
                return result;
            } break;
        }
    }

    return result;
}

const char *mp_token_to_string(MP_Token token)
{
    switch (token.type) {
        case MP_TOKEN_EOF:      return "TOKEN_EOF";
        case MP_TOKEN_INVALID:  return "TOKEN_INVALID";
        case MP_TOKEN_NUMBER:   return "TOKEN_NUMBER";
        case MP_TOKEN_SYMBOL:   return "TOKEN_SYMBOL";
        case MP_TOKEN_PLUS:     return "TOKEN_PLUS";
        case MP_TOKEN_MINUS:    return "TOKEN_MINUS";
        case MP_TOKEN_MULTIPLY: return "TOKEN_MULTIPLY";
        case MP_TOKEN_DIVIDE:   return "TOKEN_DIVIDE";
        case MP_TOKEN_POWER:    return "TOKEN_POWER";
        case MP_TOKEN_LPAREN:   return "TOKEN_LPAREN";
        case MP_TOKEN_RPAREN:   return "TOKEN_RPAREN";
        default:                return "?";
    }
}

void mp_print_token_list(MP_Token_List list)
{
    for (size_t i = 0; i < list.count; ++i) {
        MP_Token token = list.items[i];
        printf("%ld: %s", i, mp_token_to_string(token));
        if (token.type == MP_TOKEN_NUMBER) {
            printf(" %f", token.value);
        } else if (token.type == MP_TOKEN_SYMBOL) {
            printf(" %c", token.symbol);
        }
        printf("\n");
    }
}

const char *mp_error_to_string(MP_Error_Type err)
{
    switch (err) {
        case MP_ERROR_OK:                 return "No error";
        case MP_ERROR_INVALID_TOKEN:      return "Unexpected token";
        case MP_ERROR_INVALID_EXPRESSION: return "Invalid expression";
        case MP_ERROR_EMPTY_EXPRESSION:   return "Empty expression";
        case MP_ERROR_INVALID_NODE:       return "Invalid expression";
        case MP_ERROR_ZERO_DIVISION:      return "Division by zero";
        default:                          return "Unknown error";
    }
}

MP_Tree_Node *mp_make_node_binop(MP_Arena *a, MP_Node_Type t,
                                 MP_Tree_Node *lhs, MP_Tree_Node *rhs)
{
    MP_Tree_Node *r = mp_arena_alloc(a, sizeof(*r));
    r->type = t;
    r->binop.lhs = lhs;
    r->binop.rhs = rhs;
    return r;
}

MP_Tree_Node *mp_make_node_unary(MP_Arena *a, MP_Node_Type t, MP_Tree_Node *node)
{
    MP_Tree_Node *r = mp_arena_alloc(a, sizeof(*r));
    r->type = t;
    r->unary.node = node;
    return r;
}

MP_Tree_Node *mp_make_node_symbol(MP_Arena *a, char symbol)
{
    MP_Tree_Node *r = mp_arena_alloc(a, sizeof(*r));
    r->type = MP_NODE_SYMBOL;
    r->symbol = symbol;
    return r;
}

MP_Tree_Node *mp_make_node(MP_Arena *a, MP_Node_Type t, double value)
{
    MP_Tree_Node *r = mp_arena_alloc(a, sizeof(*r));
    r->type = t;
    r->value = value;
    return r;
}

MP_Result mp_parse(MP_Arena *a, MP_Parse_Tree *tree, MP_Token_List list)
{
    MP_Result result = {0};

    MP_Parser parser = {0};
    parser.tokens = list;

    mp_parser_advance(&parser);

    if (parser.current.type == MP_TOKEN_EOF) {
        result.error = true;
        result.error_type = MP_ERROR_EMPTY_EXPRESSION;
        return result;
    }

    MP_Tree_Node *tree_root = mp_parse_expr(a, &parser, &result);
    tree->root = tree_root;

    if (parser.current.type != MP_TOKEN_EOF) {
        result.error = true;
        result.error_type = MP_ERROR_INVALID_EXPRESSION;
        result.error_position = parser.current.position;
        return result;
    }

    return result;
}

void mp_parser_advance(MP_Parser *parser)
{
    if (parser->cursor >= parser->tokens.count) {
        parser->current.type = MP_TOKEN_EOF;
        return;
    }

    parser->current = parser->tokens.items[parser->cursor++];
}

MP_Tree_Node *mp_parse_expr(MP_Arena *a, MP_Parser *parser, MP_Result *result)
{
    MP_Tree_Node *result_node = mp_parse_term(a, parser, result);
    MP_Token *cur = &parser->current;

    while (!result->error
        && (cur->type == MP_TOKEN_PLUS || cur->type == MP_TOKEN_MINUS)) {

        if (cur->type == MP_TOKEN_PLUS) {
            mp_parser_advance(parser);
            result_node = mp_make_node_binop(a, MP_NODE_ADD, result_node,
                                             mp_parse_term(a, parser, result));
        } else if (cur->type == MP_TOKEN_MINUS) {
            mp_parser_advance(parser);
            result_node = mp_make_node_binop(a, MP_NODE_SUBTRACT, result_node,
                                             mp_parse_term(a, parser, result));
        }
    }

    return result_node;
}

MP_Tree_Node *mp_parse_term(MP_Arena *a, MP_Parser *parser, MP_Result *result)
{
    MP_Tree_Node *result_node = mp_parse_factor(a, parser, result);
    MP_Token *cur = &parser->current;

    while (!result->error
        && (cur->type == MP_TOKEN_MULTIPLY || cur->type == MP_TOKEN_DIVIDE
            || cur->type == MP_TOKEN_POWER)) {

        if (cur->type == MP_TOKEN_MULTIPLY) {
            mp_parser_advance(parser);
            result_node = mp_make_node_binop(a, MP_NODE_MULTIPLY, result_node,
                                             mp_parse_factor(a, parser, result));
        } else if (cur->type == MP_TOKEN_DIVIDE) {
            mp_parser_advance(parser);
            result_node = mp_make_node_binop(a, MP_NODE_DIVIDE, result_node,
                                             mp_parse_factor(a, parser, result));
        } else if (cur->type == MP_TOKEN_POWER) {
            mp_parser_advance(parser);
            result_node = mp_make_node_binop(a, MP_NODE_POWER, result_node,
                                             mp_parse_factor(a, parser, result));
        }
    }

    return result_node;
}

MP_Tree_Node *mp_parse_factor(MP_Arena *a, MP_Parser *parser, MP_Result *result)
{
    MP_Token *cur = &parser->current;

    if (cur->type == MP_TOKEN_LPAREN) {
        mp_parser_advance(parser);
        MP_Tree_Node *result_node = mp_parse_expr(a, parser, result);

        if (cur->type != MP_TOKEN_RPAREN) {
            result->error = true;
            result->error_type = MP_ERROR_INVALID_EXPRESSION;
            result->error_position = cur->position;
            return result_node;
        }

        mp_parser_advance(parser);
        return result_node;
    }

    if (cur->type == MP_TOKEN_NUMBER) {
        MP_Tree_Node *number = mp_make_node(a, MP_NODE_NUMBER, cur->value);
        mp_parser_advance(parser);
        return number;
    }

    if (cur->type == MP_TOKEN_SYMBOL) {
        MP_Tree_Node *symbol = mp_make_node_symbol(a, cur->symbol);
        mp_parser_advance(parser);
        return symbol;
    }

    if (cur->type == MP_TOKEN_PLUS) {
        mp_parser_advance(parser);
        return mp_make_node_unary(a, MP_NODE_PLUS,
                                  mp_parse_factor(a, parser, result));
    }

    if (cur->type == MP_TOKEN_MINUS) {
        mp_parser_advance(parser);
        return mp_make_node_unary(a, MP_NODE_MINUS,
                                  mp_parse_factor(a, parser, result));
    }

    result->error = true;
    result->error_type = MP_ERROR_INVALID_EXPRESSION;
    result->error_position = cur->position;
    result->faulty_token = *cur;

    return NULL;
}

void mp_print_parse_tree(MP_Parse_Tree tree)
{
    mp_print_tree_node(tree.root);
    printf("\n");
}

void mp_print_tree_node(MP_Tree_Node *root)
{
    if (root == NULL)
        return;

    switch (root->type) {
        case MP_NODE_INVALID: {
            printf("INVALID");
        } break;

        case MP_NODE_NUMBER: {
            printf("%f", root->value);
        } break;

        case MP_NODE_SYMBOL: {
            printf("%c", root->symbol);
        } break;

        case MP_NODE_ADD: {
            printf("add(");
            mp_print_tree_node(root->binop.lhs);
            printf(",");
            mp_print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case MP_NODE_SUBTRACT: {
            printf("sub(");
            mp_print_tree_node(root->binop.lhs);
            printf(",");
            mp_print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case MP_NODE_MULTIPLY: {
            printf("mul(");
            mp_print_tree_node(root->binop.lhs);
            printf(",");
            mp_print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case MP_NODE_DIVIDE: {
            printf("div(");
            mp_print_tree_node(root->binop.lhs);
            printf(",");
            mp_print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case MP_NODE_POWER: {
            printf("pow(");
            mp_print_tree_node(root->binop.lhs);
            printf(",");
            mp_print_tree_node(root->binop.rhs);
            printf(")");
        } break;

        case MP_NODE_PLUS: {
            printf("plus(");
            mp_print_tree_node(root->unary.node);
            printf(")");
        } break;

        case MP_NODE_MINUS: {
            printf("minus(");
            mp_print_tree_node(root->unary.node);
            printf(")");
        } break;

        default: {
            printf("?");
        } break;
    }
}

//-------------
// Interpreter
//-------------

MP_Result mp_interpret(MP_Parse_Tree tree)
{
    if (tree.root == NULL) {
        MP_Result r = {0};
        r.error = true;
        r.error_type = MP_ERROR_EMPTY_EXPRESSION;
        return r;
    }

    return mp_interpret_node(tree.root);
}

MP_Result mp_interpret_node(MP_Tree_Node *root)
{
    MP_Result result = {0};

    if (root == NULL) {
        result.error = true;
        result.error_type = MP_ERROR_INVALID_NODE;
        return result;
    }

    switch (root->type) {
        case MP_NODE_INVALID: {
            result.error = true;
            result.error_type = MP_ERROR_INVALID_NODE;
            return result;
        } break;

        case MP_NODE_NUMBER: {
            result.value = root->value;
        } break;

        case MP_NODE_ADD: {
            MP_Result a = mp_interpret_node(root->binop.lhs);
            if (a.error) return a;
            MP_Result b = mp_interpret_node(root->binop.rhs);
            if (b.error) return b;
            result.value = a.value + b.value;
        } break;

        case MP_NODE_SUBTRACT: {
            MP_Result a = mp_interpret_node(root->binop.lhs);
            if (a.error) return a;
            MP_Result b = mp_interpret_node(root->binop.rhs);
            if (b.error) return b;
            result.value = a.value - b.value;
        } break;

        case MP_NODE_MULTIPLY: {
            MP_Result a = mp_interpret_node(root->binop.lhs);
            if (a.error) return a;
            MP_Result b = mp_interpret_node(root->binop.rhs);
            if (b.error) return b;
            result.value = a.value * b.value;
        } break;

        case MP_NODE_DIVIDE: {
            MP_Result b = mp_interpret_node(root->binop.rhs);
            if (b.error) return b;
            if (b.value == 0.0) {
                result.error = true;
                result.error_type = MP_ERROR_ZERO_DIVISION;
                return result;
            }

            MP_Result a = mp_interpret_node(root->binop.lhs);
            if (a.error) return a;
            result.value = a.value / b.value;
        } break;

        case MP_NODE_POWER: {
            MP_Result b = mp_interpret_node(root->binop.rhs);
            if (b.error) return b;
            MP_Result a = mp_interpret_node(root->binop.lhs);
            if (a.error) return a;
            result.value = pow(a.value, b.value);
        } break;

        case MP_NODE_PLUS: {
            result = mp_interpret_node(root->unary.node);
        } break;

        case MP_NODE_MINUS: {
            MP_Result n = mp_interpret_node(root->unary.node);
            result.value = -n.value;
        } break;

        default: {
            result.error = true;
            result.error_type = MP_ERROR_INVALID_NODE;
            return result;
        } break;
    }

    return result;
}

//----------
// Compiler
//----------

bool mp_program_compile(MP_Program *p, MP_Parse_Tree parse_tree)
{
    return mp_program_compile_node(p, parse_tree.root);
}

bool mp_program_compile_node(MP_Program *p, MP_Tree_Node *node)
{
    switch (node->type) {
        case MP_NODE_INVALID: {
            return false;
        } break;

        case MP_NODE_NUMBER: {
            mp_program_push_opcode(p, MP_OP_PUSH_NUM);
            mp_program_push_const(p, node->value);
        } break;

        case MP_NODE_SYMBOL: {
            assert('a' <= node->symbol && node->symbol <= 'z');
            mp_program_push_opcode(p, MP_OP_PUSH_VAR);
            mp_program_push_var(p, node->symbol - 'a');
        } break;

        case MP_NODE_ADD: {
            if (!mp_program_compile_node(p, node->binop.lhs)) return false;
            if (!mp_program_compile_node(p, node->binop.rhs)) return false;
            mp_program_push_opcode(p, MP_OP_ADD);
        } break;

        case MP_NODE_SUBTRACT: {
            if (!mp_program_compile_node(p, node->binop.lhs)) return false;
            if (!mp_program_compile_node(p, node->binop.rhs)) return false;
            mp_program_push_opcode(p, MP_OP_SUB);
        } break;

        case MP_NODE_MULTIPLY: {
            if (!mp_program_compile_node(p, node->binop.lhs)) return false;
            if (!mp_program_compile_node(p, node->binop.rhs)) return false;
            mp_program_push_opcode(p, MP_OP_MUL);
        } break;

        case MP_NODE_DIVIDE: {
            if (!mp_program_compile_node(p, node->binop.lhs)) return false;
            if (!mp_program_compile_node(p, node->binop.rhs)) return false;
            mp_program_push_opcode(p, MP_OP_DIV);
        } break;

        case MP_NODE_POWER: {
            if (!mp_program_compile_node(p, node->binop.lhs)) return false;
            if (!mp_program_compile_node(p, node->binop.rhs)) return false;
            mp_program_push_opcode(p, MP_OP_POW);
        } break;

        case MP_NODE_PLUS: {
            if (!mp_program_compile_node(p, node->unary.node)) return false;
        } break;

        case MP_NODE_MINUS: {
            if (!mp_program_compile_node(p, node->unary.node)) return false;
            mp_program_push_opcode(p, MP_OP_NEG);
        } break;

        default: {
            return false;
        } break;
    }

    return true;
}

void mp_program_push_opcode(MP_Program *p, MP_Opcode op)
{
    mp_da_append(p, op);
}

void mp_program_push_const(MP_Program *p, double value)
{
    for (size_t i = 0; i < sizeof(value); ++i) {
        mp_da_append(p, 0);
    }
    double *loc = (double*)((p->items + p->count) - sizeof(*loc));
    *loc = value;
}

void mp_program_push_var(MP_Program *p, char var)
{
    mp_da_append(p, var);
}

void mp_print_program(MP_Program p)
{
    size_t ip = 0;
    for (size_t i = 0; i < p.count; ++i) {
        MP_Opcode op = p.items[i];

        switch (op) {
            case MP_OP_PUSH_NUM: {
                printf("%ld: PUSH_NUM ", ip++);
                double num = 0.0;

                if (i + sizeof(num) >= p.count)
                    continue;

                ++i;
                num = *(double*)&p.items[i];
                i += sizeof(num) - 1;

                printf("%f\n", num);
            } break;

            case MP_OP_PUSH_VAR: {
                printf("%ld: PUSH_VAR ", ip++);
                char var = 0;

                if (i + sizeof(var) >= p.count)
                    continue;

                ++i;
                var = p.items[i] + 'a';
                // i += sizeof(var) - 1; // Increment by 0

                printf("%c\n", var);
            } break;

            case MP_OP_ADD: printf("%ld: ADD\n", ip++); break;
            case MP_OP_SUB: printf("%ld: SUB\n", ip++); break;
            case MP_OP_MUL: printf("%ld: MUL\n", ip++); break;
            case MP_OP_DIV: printf("%ld: DIV\n", ip++); break;
            case MP_OP_POW: printf("%ld: POW\n", ip++); break;
            case MP_OP_NEG: printf("%ld: NEG\n", ip++); break;

            default: {
                printf("%ld: ?\n", ip++);
            } break;
        }
    }
}

void mp_stack_push(MP_Stack *stack, double n)
{
    mp_da_append(stack, n);
}

MP_Optional mp_stack_pop(MP_Stack *stack)
{
    MP_Optional result = mp_stack_peek(stack);
    if (result.present) {
        stack->count--;
    }

    return result;
}

MP_Optional mp_stack_peek(MP_Stack *stack)
{
    MP_Optional result = {0};
    if (stack->count == 0) {
        return result;
    }

    result.present = true;
    result.value = stack->items[stack->count - 1];
    return result;
}

MP_Vm mp_vm_init(MP_Program program)
{
    MP_Vm vm = {0};
    vm.program = program;
    return vm;
}

void mp_vm_var(MP_Vm *vm, char var, double value)
{
    assert('a' <= var && var <= 'z');
    vm->vars[var - 'a'] = value;
}

bool mp_vm_run(MP_Vm *vm)
{
#define ASSERT_PRESENT(o) if (!(o).present) return false

    MP_Stack *stack = &vm->stack;
    MP_Program *program = &vm->program;
    vm->ip = 0;

    while (vm->ip < program->count) {
        MP_Opcode op = program->items[vm->ip];

        switch (op) {
            case MP_OP_PUSH_NUM: {
                ++vm->ip;
                double operand = *(double*)&program->items[vm->ip];
                mp_stack_push(stack, operand);
                vm->ip += sizeof(operand);
            } break;

            case MP_OP_PUSH_VAR: {
                ++vm->ip;
                char var = *(char*)&program->items[vm->ip];
                mp_stack_push(stack, vm->vars[(int)var]);
                vm->ip += sizeof(var);
            } break;

            case MP_OP_ADD: {
                MP_Optional b = mp_stack_pop(stack); ASSERT_PRESENT(b);
                MP_Optional a = mp_stack_pop(stack); ASSERT_PRESENT(a);
                mp_stack_push(stack, a.value + b.value);
                ++vm->ip;
            } break;

            case MP_OP_SUB: {
                MP_Optional b = mp_stack_pop(stack); ASSERT_PRESENT(b);
                MP_Optional a = mp_stack_pop(stack); ASSERT_PRESENT(a);
                mp_stack_push(stack, a.value - b.value);
                ++vm->ip;
            } break;

            case MP_OP_MUL: {
                MP_Optional b = mp_stack_pop(stack); ASSERT_PRESENT(b);
                MP_Optional a = mp_stack_pop(stack); ASSERT_PRESENT(a);
                mp_stack_push(stack, a.value * b.value);
                ++vm->ip;
            } break;

            case MP_OP_DIV: {
                MP_Optional b = mp_stack_pop(stack); ASSERT_PRESENT(b);
                MP_Optional a = mp_stack_pop(stack); ASSERT_PRESENT(a);
                mp_stack_push(stack, a.value / b.value);
                ++vm->ip;
            } break;

            case MP_OP_POW: {
                MP_Optional b = mp_stack_pop(stack); ASSERT_PRESENT(b);
                MP_Optional a = mp_stack_pop(stack); ASSERT_PRESENT(a);
                mp_stack_push(stack, pow(a.value, b.value));
                ++vm->ip;
            } break;

            case MP_OP_NEG: {
                MP_Optional n = mp_stack_pop(stack); ASSERT_PRESENT(n);
                mp_stack_push(stack, -n.value);
                ++vm->ip;
            } break;

            default: {
                return false;
            } break;
        }
    }

    return true;

#undef ASSERT_PRESENT
}

double mp_vm_result(MP_Vm *vm)
{
    return mp_stack_peek(&vm->stack).value;
}

void mp_vm_free(MP_Vm *vm)
{
    mp_da_free(&vm->stack);
}

//-----------------------------------
// Simplified API (uses compilation)
//-----------------------------------

MP_Env *mp_init(const char *expression)
{
    MP_Env *env = malloc(sizeof(*env));
    if (env == NULL) {
        return NULL;
    }
    memset(env, 0, sizeof(*env));

    MP_Token_List token_list = {0};

    MP_Result tr = mp_tokenize(&token_list, expression);
    if (tr.error) {
        mp_da_free(&token_list);
        return NULL;
    }

    MP_Arena arena = {0};
    MP_Parse_Tree parse_tree = {0};

    MP_Result pr = mp_parse(&arena, &parse_tree, token_list);
    if (pr.error) {
        mp_da_free(&token_list);
        mp_arena_free(&arena);
        return NULL;
    }

    if (!mp_program_compile(&env->program, parse_tree)) {
        mp_da_free(&token_list);
        mp_arena_free(&arena);
        mp_da_free(&env->program);
        return NULL;
    }

    mp_da_free(&token_list);
    mp_arena_free(&arena);

    env->vm = mp_vm_init(env->program);

    return env;
}

void mp_variable(MP_Env *env, char var, double value)
{
    mp_vm_var(&env->vm, var, value);
}

MP_Result mp_evaluate(MP_Env *env)
{
    MP_Result result = {0};

    if (!mp_vm_run(&env->vm)) {
        result.error = true;
        return result;
    }

    result.value = mp_vm_result(&env->vm);
    return result;
}

void mp_free(MP_Env *env)
{
    mp_da_free(&env->program);
    mp_vm_free(&env->vm);
}

#endif // MP_IMPLEMENTATION

/*
    Revision history:

        1.1.0 (2025-03-12) Implement exponentiation
        1.0.2 (2025-03-12) Remove unused macro
        1.0.1 (2025-03-12) Fix inconsistency of MP_Env memory on initialization
        1.0.0 (2025-03-12) Initial release
*/

/*
 * MIT License
 * 
 * Copyright (c) 2025 seajee
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
