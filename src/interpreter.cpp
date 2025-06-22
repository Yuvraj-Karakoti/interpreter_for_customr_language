#include "interpreter.h"

interpreter::interpreter(std::vector <token> &_tokens) {
    tokens = _tokens;
    std::vector <std::pair<int, int>> blocks = ast_node::gen_blocks(tokens, false);
    bool fn_declared = false;
    bool main_declared = false;

    for (int i = 0; i < blocks.size(); ++i) {
        if (!fn_declared && token::vars.find(tokens[blocks[i].first].val) != token::vars.end())
            interpreter::declare_obj(ast_node::subarray(tokens, blocks[i].first, blocks[i].second), true);
        else if (!main_declared && tokens[blocks[i].first].val == "fn") {
            fn_declared = true;
            main_declared = declare_fn(blocks[i].first, blocks[i].second);
        } else
            err("invalid sequence", tokens[blocks[i].first].line);
    }

    if (!main_declared)
        err("main function not declared");
}

void interpreter::declare_obj(std::vector <token> obj, bool to_global) {
    if (obj.size() != 2)
        err("variable declaration format is [type] [identifier]", obj.back().line);
    if (obj.back().type != t_symbol)
        err("invalid variable identifier", obj.back().line);
    if (!memory::valid(obj.back().val))
        err("cannot redeclare existing symbol \"" + obj.back().val + "\"", obj.back().line);
    o_type t_obj = object::str_o_type(obj.front().val);
    std::variant<double, std::string, bool, std::vector<object *>, std::queue<object *>, std::stack<object *>, std::unordered_set<object *, obj_hash, obj_equals>, std::unordered_map<object *, object *, obj_hash, obj_equals>> store;
    if (obj.front().val == "num")
        store = (double) 0;
    else if (obj.front().val == "bool")
        store = false;
    else if (obj.front().val == "str")
        store = "";
    else if (obj.front().val == "arr")
        store = std::vector<object *>();
    else if (obj.front().val == "queue")
        store = std::queue<object *>();
    else if (obj.front().val == "stack")
        store = std::stack<object *>();
    else if (obj.front().val == "set")
        store = std::unordered_set<object *, obj_hash, obj_equals>();
    else if (obj.front().val == "map")
        store = std::unordered_map<object *, object *, obj_hash, obj_equals>();
    else
        err("unimplemented var type");

    object *tmp = new object(t_obj);
    tmp->set(store);
    memory::add(obj.back().val, tmp, to_global);
}

bool interpreter::declare_fn(int start, int end) {
    int beg = start + 1;
    if (end - start < 7)
        err("function declaration is too short", tokens[end].line);
    if (tokens[start].val == "fn" &&
        (tokens[++start].val == "main" || memory::valid(tokens[start].val)) &&
        token::vars.find(tokens[++start].val) != token::vars.end()) {
        object *fn_obj = new object(o_fn);
        fn_obj->f_return = object::str_o_type(tokens[start].val);
        ++start;
        if (tokens[start].val != "(")
            err("invalid function declaration format", tokens[start].line);
        int p_end = start;
        while (p_end < end && tokens[p_end].val != "start" && tokens[p_end].val != ")")
            ++p_end;
        if (p_end == end)
            err("invalid parameter declaration", tokens[end].line);
        if (tokens[p_end].val == "start")
            err(") not found in function declaration", tokens[p_end].line);
        int p_start = ++start;
        --p_end;
        std::unordered_set <std::string> used_symbol;
        while (tokens[start].val != ")") {
            if (tokens[start].val == ",") {
                if (start - p_start != 2)
                    err("invalid parameter declaration", tokens[start].line);
                if (token::vars.find(tokens[p_start].val) == token::vars.end() || tokens[p_start].val == "none")
                    err("parameter declaration must define type", tokens[start].line);
                if (tokens[p_start + 1].type != t_symbol)
                    err("parameter identifier invalid", tokens[start].line);
                if (used_symbol.find(tokens[p_start + 1].val) != used_symbol.end())
                    err("parameter name used twice", tokens[start].line);
                used_symbol.insert(tokens[p_start + 1].val);
                fn_obj->f_params.emplace_back(object::str_o_type(tokens[p_start].val), tokens[p_start + 1].val);
                p_start = start + 1;
            }
            ++start;
        }
        if (tokens[start].val != ")" ||
            (start == p_start && !(tokens[start - 1].val == "(" && tokens[start].val == ")")))
            err("invalid function declaration parentheses", tokens[start].line);
        if (!(tokens[start - 1].val == "(" && tokens[start].val == ")")) {
            if (start - p_start != 2)
                err("invalid parameter declaration", tokens[start].line);
            if (token::vars.find(tokens[p_start].val) == token::vars.end() || tokens[p_start].val == "none")
                err("parameter declaration must define type", tokens[start].line);
            if (tokens[p_start + 1].type != t_symbol)
                err("parameter identifier invalid", tokens[start].line);
            if (used_symbol.find(tokens[p_start + 1].val) != used_symbol.end())
                err("parameter name used twice", tokens[start].line);
            used_symbol.insert(tokens[p_start + 1].val);
            fn_obj->f_params.emplace_back(object::str_o_type(tokens[p_start].val), tokens[p_start + 1].val);
        }
        ++start;
        if (tokens[start].val != "start")
            err("function block must begin after parameters", tokens[start].line);
        fn_obj->f_body = new ast_node(ast_node::subarray(tokens, start + 2, end - 1));
        memory::add(tokens[beg].val, fn_obj, true);
        return tokens[beg].val == "main";
    } else
        err("invalid function declaration", tokens[start].line);
    return false;
}

void interpreter::execute() {
    object *root = memory::get("main");
    if (root->f_params.size() != 0)
        err("main must have no parameters");
    if (root->f_return != o_none)
        err("main must have return type none");
    memory::push();
    executor *process = new executor(root->f_body, root);
    auto start = std::chrono::high_resolution_clock::now();
    process->init();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    memory::pop();
}
