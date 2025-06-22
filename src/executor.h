
#ifndef QI_INTERPRETER_EXECUTOR_H
#define QI_INTERPRETER_EXECUTOR_H

#include <vector>

#include "ast_node.h"
#include "interpreter.h"
#include "memory.h"
#include "object.h"
#include "token.h"
#include "util.h"


class executor {
private:
    ast_node *tree;
    object *parent;
    bool has_return, has_continue, has_break;
    object *return_val;

public:
    executor(ast_node *_tree, object *_parent);

    object *init();

    object *run(ast_node *u);
};

#endif 
