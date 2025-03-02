#ifndef VM_H_
#define VM_H_

#include <stddef.h>

#include "parser.h"

typedef enum {
    INST_INVALID,
    INST_PUSH,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_NEG,
    INST_COUNT
} Inst_Opcode;

typedef struct {
    Inst_Opcode type;
    double value;
} Inst;

// TODO: Compile down to bytecode instead of making an array of instructions
typedef struct {
    size_t count;
    size_t capacity;
    Inst *items;
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

const char *inst_to_string(Inst inst);
Inst make_inst(Inst_Opcode type);
Inst make_inst_push(double value);

Program program_compile(Parse_Tree parse_tree);
bool compile_node(Tree_Node *node, Program *program);
void print_program(Program p);

void stack_push(Stack *stack, double n);
double stack_pop(Stack *stack);
double stack_peek(Stack *stack);

Vm vm_init(Program program);
bool vm_run(Vm *vm);
double vm_result(const Vm *vm);
void vm_free(Vm *vm);

#endif // VM_H_
