#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "parser.h"

Result interpret(Parse_Tree tree);
Result interpret_node(Tree_Node *root);

#endif // INTERPRETER_H_
