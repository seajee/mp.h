#ifndef VM_H_
#define VM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "parser.h"

typedef enum {
    OP_INVALID,
    OP_PUSH,
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
    size_t ip;
} Vm;

typedef struct {
    bool present;
    double value;
} Optional;

Program program_compile(Parse_Tree parse_tree); // TODO: Handle compilation errors
bool program_compile_node(Program *p, Tree_Node *node); // TODO: Detailed errors
void program_push_opcode(Program *p, Opcode op);
void program_push_operand(Program *p, double value);
void print_program(Program p);

void stack_push(Stack *stack, double n);
Optional stack_pop(Stack *stack);
Optional stack_peek(Stack *stack);

Vm vm_init(Program program);
bool vm_run(Vm *vm); // TODO: Detailed errors
double vm_result(Vm *vm);
void vm_free(Vm *vm);

#endif // VM_H_
