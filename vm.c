#include "vm.h"

#include <assert.h>
#include <stdio.h>

#include "util.h"

const char *inst_to_string(Inst inst)
{
    switch (inst.type) {
        case INST_INVALID: return "INST_INVALID";
        case INST_PUSH:    return "INST_PUSH";
        case INST_ADD:     return "INST_ADD";
        case INST_SUB:     return "INST_SUB";
        case INST_MUL:     return "INST_MUL";
        case INST_DIV:     return "INST_DIV";
        case INST_NEG:     return "INST_NEG";
        default: {
            UNREACHABLE();
        } break;
    }
}

Inst make_inst(Inst_Opcode type)
{
    Inst r = {0};
    r.type = type;
    return r;
}

Inst make_inst_push(double value)
{
    Inst r = make_inst(INST_PUSH);
    r.value = value;
    return r;
}

Program program_compile(Parse_Tree parse_tree)
{
    Program program = {0};
    compile_node(parse_tree.root, &program); // TODO: Handle compilation errors
    return program;
}

bool compile_node(Tree_Node *node, Program *program)
{
    switch (node->type) {
        case NODE_INVALID: {
            return false;
        } break;

        case NODE_NUMBER: {
            Inst op = make_inst_push(node->value);
            da_append(program, op);
        } break;

        case NODE_ADD: {
            if (!compile_node(node->binop.lhs, program)) return false;
            if (!compile_node(node->binop.rhs, program)) return false;

            Inst op = make_inst(INST_ADD);
            da_append(program, op);
        } break;

        case NODE_SUBTRACT: {
            if (!compile_node(node->binop.lhs, program)) return false;
            if (!compile_node(node->binop.rhs, program)) return false;

            Inst op = make_inst(INST_SUB);
            da_append(program, op);
        } break;

        case NODE_MULTIPLY: {
            if (!compile_node(node->binop.lhs, program)) return false;
            if (!compile_node(node->binop.rhs, program)) return false;

            Inst op = make_inst(INST_MUL);
            da_append(program, op);
        } break;

        case NODE_DIVIDE: {
            if (!compile_node(node->binop.lhs, program)) return false;
            if (!compile_node(node->binop.rhs, program)) return false;

            Inst op = make_inst(INST_DIV);
            da_append(program, op);
        } break;

        case NODE_PLUS: {
            if (!compile_node(node->unary.node, program)) return false;
        } break;

        case NODE_MINUS: {
            if (!compile_node(node->unary.node, program)) return false;
            Inst op = make_inst(INST_NEG);
            da_append(program, op);
        } break;

        default: {
            UNREACHABLE();
        } break;
    }

    return true;
}

void print_program(Program p)
{
    for (size_t i = 0; i < p.count; ++i) {
        Inst op = p.items[i];
        printf("%ld: Inst %s, value %f\n", i, inst_to_string(op), op.value);
    }
}

void stack_push(Stack *stack, double n)
{
    da_append(stack, n);
}

double stack_pop(Stack *stack)
{
    // TODO: Check if stack is empty
    double r = stack->items[stack->count - 1];
    stack->count--;
    return r;
}

double stack_peek(Stack *stack)
{
    assert(stack->items != NULL);
    return stack->items[stack->count - 1];
}

Vm vm_init(Program program)
{
    Vm vm = {0};
    vm.program = program;
    return vm;
}

bool vm_run(Vm *vm)
{
    Stack *stack = &vm->stack;
    Program *program = &vm->program;

    while (vm->ip < program->count) {
        Inst *inst = &program->items[vm->ip];

        switch (inst->type) {
            case INST_INVALID: {
                // TODO: Handle invalid opcode. Skipping for now
            } break;

            case INST_PUSH: {
                stack_push(stack, inst->value);
            } break;

            case INST_ADD: {
                double b = stack_pop(stack);
                double a = stack_pop(stack);
                stack_push(stack, a + b);
            } break;

            case INST_SUB: {
                double b = stack_pop(stack);
                double a = stack_pop(stack);
                stack_push(stack, a - b);
            } break;

            case INST_MUL: {
                double b = stack_pop(stack);
                double a = stack_pop(stack);
                stack_push(stack, a * b);
            } break;

            case INST_DIV: {
                double b = stack_pop(stack);
                double a = stack_pop(stack);
                stack_push(stack, a / b);
            } break;

            case INST_NEG: {
                double n = stack_pop(stack);
                stack_push(stack, -n);
            } break;

            default: {
                // TODO: Handle unknown opcode. Skipping for now
            } break;
        }

        vm->ip++;
    }

    return true;
}

double vm_result(const Vm *vm)
{
    // TODO: Handle empty stack correctly
    assert(vm->stack.count == 1);
    return vm->stack.items[0];
}

void vm_free(Vm *vm)
{
    da_free(&vm->program);
    da_free(&vm->stack);
}
