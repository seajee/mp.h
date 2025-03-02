// TODO: Give more detailed errors

#include "vm.h"

#include <assert.h>
#include <stdio.h>

#include "util.h"

bool program_compile(Program *p, Parse_Tree parse_tree)
{
    return program_compile_node(p, parse_tree.root);
}

bool program_compile_node(Program *p, Tree_Node *node)
{
    switch (node->type) {
        case NODE_INVALID: {
            return false;
        } break;

        case NODE_NUMBER: {
            program_push_opcode(p, OP_PUSH_NUM);
            program_push_const(p, node->value);
        } break;

        case NODE_SYMBOL: {
            assert('a' <= node->symbol && node->symbol <= 'z');
            program_push_opcode(p, OP_PUSH_VAR);
            program_push_var(p, node->symbol - 'a');
        } break;

        case NODE_ADD: {
            if (!program_compile_node(p, node->binop.lhs)) return false;
            if (!program_compile_node(p, node->binop.rhs)) return false;
            program_push_opcode(p, OP_ADD);
        } break;

        case NODE_SUBTRACT: {
            if (!program_compile_node(p, node->binop.lhs)) return false;
            if (!program_compile_node(p, node->binop.rhs)) return false;
            program_push_opcode(p, OP_SUB);
        } break;

        case NODE_MULTIPLY: {
            if (!program_compile_node(p, node->binop.lhs)) return false;
            if (!program_compile_node(p, node->binop.rhs)) return false;
            program_push_opcode(p, OP_MUL);
        } break;

        case NODE_DIVIDE: {
            if (!program_compile_node(p, node->binop.lhs)) return false;
            if (!program_compile_node(p, node->binop.rhs)) return false;
            program_push_opcode(p, OP_DIV);
        } break;

        case NODE_PLUS: {
            if (!program_compile_node(p, node->unary.node)) return false;
        } break;

        case NODE_MINUS: {
            if (!program_compile_node(p, node->unary.node)) return false;
            program_push_opcode(p, OP_NEG);
        } break;

        default: {
            return false;
        } break;
    }

    return true;
}

void program_push_opcode(Program *p, Opcode op)
{
    da_append(p, op);
}

void program_push_const(Program *p, double value)
{
    for (size_t i = 0; i < sizeof(value); ++i) {
        da_append(p, 0);
    }
    double *loc = (double*)((p->items + p->count) - sizeof(*loc));
    *loc = value;
}

void program_push_var(Program *p, char var)
{
    da_append(p, var);
}

void print_program(Program p)
{
    size_t ip = 0;
    for (size_t i = 0; i < p.count; ++i) {
        Opcode op = p.items[i];

        switch (op) {
            case OP_PUSH_NUM: {
                printf("%ld: PUSH_NUM ", ip++);
                double num = 0.0;

                if (i + sizeof(num) >= p.count)
                    continue;

                ++i;
                num = *(double*)&p.items[i];
                i += sizeof(num) - 1;

                printf("%f\n", num);
            } break;

            case OP_PUSH_VAR: {
                printf("%ld: PUSH_VAR ", ip++);
                char var = 0;

                if (i + sizeof(var) >= p.count)
                    continue;

                ++i;
                var = p.items[i] + 'a';
                // i += sizeof(var) - 1; // Increment by 0

                printf("%c\n", var);
            } break;

            case OP_ADD: printf("%ld: ADD\n", ip++); break;
            case OP_SUB: printf("%ld: SUB\n", ip++); break;
            case OP_MUL: printf("%ld: MUL\n", ip++); break;
            case OP_DIV: printf("%ld: DIV\n", ip++); break;
            case OP_NEG: printf("%ld: NEG\n", ip++); break;

            default: {
                printf("%ld: ?\n", ip++);
            } break;
        }
    }
}

void stack_push(Stack *stack, double n)
{
    da_append(stack, n);
}

Optional stack_pop(Stack *stack)
{
    Optional result = stack_peek(stack);
    if (result.present) {
        stack->count--;
    }

    return result;
}

Optional stack_peek(Stack *stack)
{
    Optional result = {0};
    if (stack->count == 0) {
        return result;
    }

    result.present = true;
    result.value = stack->items[stack->count - 1];
    return result;
}

Vm vm_init(Program program)
{
    Vm vm = {0};
    vm.program = program;
    return vm;
}

bool vm_run(Vm *vm)
{
#define ASSERT_PRESENT(o) if (!(o).present) return false

    Stack *stack = &vm->stack;
    Program *program = &vm->program;
    vm->ip = 0;

    while (vm->ip < program->count) {
        Opcode op = program->items[vm->ip];

        switch (op) {
            case OP_PUSH_NUM: {
                ++vm->ip;
                double operand = *(double*)&program->items[vm->ip];
                stack_push(stack, operand);
                vm->ip += sizeof(operand);
            } break;

            case OP_PUSH_VAR: {
                ++vm->ip;
                char var = *(char*)&program->items[vm->ip];
                stack_push(stack, vm->vars[(int)var]);
                vm->ip += sizeof(var);
            } break;

            case OP_ADD: {
                Optional b = stack_pop(stack); ASSERT_PRESENT(b);
                Optional a = stack_pop(stack); ASSERT_PRESENT(a);
                stack_push(stack, a.value + b.value);
                ++vm->ip;
            } break;

            case OP_SUB: {
                Optional b = stack_pop(stack); ASSERT_PRESENT(b);
                Optional a = stack_pop(stack); ASSERT_PRESENT(a);
                stack_push(stack, a.value - b.value);
                ++vm->ip;
            } break;

            case OP_MUL: {
                Optional b = stack_pop(stack); ASSERT_PRESENT(b);
                Optional a = stack_pop(stack); ASSERT_PRESENT(a);
                stack_push(stack, a.value * b.value);
                ++vm->ip;
            } break;

            case OP_DIV: {
                Optional b = stack_pop(stack); ASSERT_PRESENT(b);
                Optional a = stack_pop(stack); ASSERT_PRESENT(a);
                stack_push(stack, a.value / b.value);
                ++vm->ip;
            } break;

            case OP_NEG: {
                Optional n = stack_pop(stack); ASSERT_PRESENT(n);
                stack_push(stack, -n.value);
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

double vm_result(Vm *vm)
{
    return stack_peek(&vm->stack).value;
}

void vm_free(Vm *vm)
{
    da_free(&vm->stack);
}
