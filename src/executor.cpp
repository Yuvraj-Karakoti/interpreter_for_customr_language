#include "executor.h"

executor::executor(ast_node *_tree, object *_parent) {
    tree = _tree;
    parent = _parent;
}

object *executor::init() {
    has_return = false;
    has_continue = false;
    has_break = false;
    object *res = run(tree);
    if (has_return && parent->f_return == o_none)
        err("none function returned non-none object");
    if (!has_return && parent->f_return != o_none)
        err("non-none function returned none");
    if (has_return && return_val->type != parent->f_return)
        err("function return type does not match returned object type");
    if (has_continue)
        err("continue called outside loop");
    if (has_break)
        err("break called outside loop");
    return has_return ? return_val : res;
}
