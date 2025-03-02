#ifndef VM_H_
#define VM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "parser.h"

typedef enum {
    OP_INVALID,
    OP_PUSH_NUM,
    OP_PUSH_VAR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_COUNT
} Opcode;

typedef struct {
    size_t count;
    size_t capacity;
    uint8_t *items;
} Program;

typedef struct {
    size_t count;
    size_t capacity;
    double *items;
} Stack;

typedef struct {
    Program program;
    Stack stack;
    double vars[26]; // a - z
    size_t ip;
} Vm;

typedef struct {
    bool present;
    double value;
} Optional;

bool program_compile(Program *p, Parse_Tree parse_tree);
bool program_compile_node(Program *p, Tree_Node *node);
void program_push_opcode(Program *p, Opcode op);
void program_push_const(Program *p, double value);
void program_push_var(Program *p, char var);
void print_program(Program p);

void stack_push(Stack *stack, double n);
Optional stack_pop(Stack *stack);
Optional stack_peek(Stack *stack);

Vm vm_init(Program program);
void vm_var(Vm *vm, char var, double value);
bool vm_run(Vm *vm);
double vm_result(Vm *vm);
void vm_free(Vm *vm);

#endif // VM_H_
