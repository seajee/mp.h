#include "interpreter.h"
#include "util.h"

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
            result.error = true;
            result.error_type = ERROR_INVALID_NODE;
            return result;
        } break;
    }

    return result;
}
