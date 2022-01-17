#include <iostream>
#include <vector>
#include <stack>
#include "generation.h"

using namespace std;

stack<AnalyzingStackNode> analyzing_stack;  // 分析栈
JumpInsList jump_ins_list;                  // 跳转列表

// 每个节点对应的label值
map<SyntaxTreeNode *, int> label_start;
map<SyntaxTreeNode *, int> label_end;
map<SyntaxTreeNode *, int> label_else;

// 将中间代码加入跳转列表并返回加入位置
int push_list(MiddleCodeType mcode_type, const string &result_str = "", const string &first_arg = "", const string &second_arg = "") {
    jump_ins_list.middle_codes.push_back(MiddleCode(mcode_type, first_arg, second_arg, result_str));
    return jump_ins_list.middle_codes.size() - 1;
}

// 生成中间代码
void generate_middle_code(SyntaxTreeNode *root, int & label_num, int & t_num) {
    // 如果语法树的根结点为空，直接返回
    if (root == nullptr) {
        return;
    }

    SyntaxTreeNode *child1 = root->child[0];
    SyntaxTreeNode *child2 = root->child[1];
    SyntaxTreeNode *child3 = root->child[2];

    // 拉链回填儿子结点
    SyntaxTreeNode *backpatching_child1;
    SyntaxTreeNode *backpatching_child2;

    string first_arg, second_arg, result_str, temp;
    // 拉链回填序号
    int backpatching_index;
    // 拉链回填合并列表
    vector<int> merge_list;

    // 对于repeat语句，前面要加label
    if (root->node_type == REPEAT_STMT) {
        push_list(MID_CODE_TYPE_LABEL, to_string(++label_num));
        label_start[root] = label_num;
    }
    // 递归生成中间代码
    generate_middle_code(child1, label_num, t_num);

    // 对于if语句，它的then和else语句加label
    if (root->node_type == IF_STMT || root->node_type == WHILE_STMT) {
        push_list(MID_CODE_TYPE_LABEL, to_string(++label_num));
        label_start[child2] = label_num;
    }
    // 递归生成中间代码
    generate_middle_code(child2, label_num, t_num);

    if (root->node_type == IF_STMT) {
        // 有else子句那么在then子句后面加goto并且回填，使else子句不要紧跟then后面执行
        if (child3) {
            int next_index = push_list(MID_CODE_TYPE_GOTO, "dest");
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_NEXT_LIST, root, next_index);
        }
        push_list(MID_CODE_TYPE_LABEL, to_string(++label_num));
        label_else[root] = label_num;
    }
    // 对于while语句，它的执行语句后面要加goto，指向判断前面，后面加上label
    else if (root->node_type == WHILE_STMT) {
        //WHILE语句需要再执行语句紧接一个GOTO，指向判断前面
        int next_index = push_list(MID_CODE_TYPE_GOTO, "dest");
        jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_NEXT_LIST, child2, next_index);

        push_list(MID_CODE_TYPE_LABEL, to_string(++label_num));
        label_end[root] = label_num;
    }
    // 对于repeat语句，它的后面要加label
    else if (root->node_type == REPEAT_STMT) {
        push_list(MID_CODE_TYPE_LABEL, to_string(++label_num));
        label_end[root] = label_num;
    }
    // 递归生成中间代码
    generate_middle_code(child3, label_num, t_num);
    if (root->node_type == IF_STMT) {
        // 对于else子句，后面加一个label，then子句的next指向这个label
        if (child3) {
            push_list(MID_CODE_TYPE_LABEL, to_string(++label_num));
            jump_ins_list.backpatching(JUMP_INS_TYPE_NEXT_LIST, root, label_num);
        }
    }

    int node_type = root->node_type;
    switch (node_type) {
        case WHILE_STMT:
            // 获取回填儿子结点
            backpatching_child1 = root->child[0];
            backpatching_child2 = root->child[1];

            // 进行回填
            jump_ins_list.backpatching(JUMP_INS_TYPE_TRUE_LIST, backpatching_child1, label_start[backpatching_child1]);
            jump_ins_list.backpatching(JUMP_INS_TYPE_FALSE_LIST, backpatching_child1, label_end[root]);
            jump_ins_list.backpatching(JUMP_INS_TYPE_NEXT_LIST, backpatching_child2, label_start[backpatching_child1]);
            break;
        case IF_STMT:
            // 获取回填儿子结点
            backpatching_child1 = root->child[0];
            backpatching_child2 = root->child[1];

            // 进行回填
            jump_ins_list.backpatching(JUMP_INS_TYPE_TRUE_LIST, backpatching_child1, label_start[backpatching_child2]);
            jump_ins_list.backpatching(JUMP_INS_TYPE_FALSE_LIST, backpatching_child1, label_else[root]);
            break;
        case REPEAT_STMT:
            // 获取回填儿子结点
            backpatching_child1 = root->child[0];
            backpatching_child2 = root->child[1];

            // 进行回填
            jump_ins_list.backpatching(JUMP_INS_TYPE_FALSE_LIST, backpatching_child2, label_start[root]);
            jump_ins_list.backpatching(JUMP_INS_TYPE_TRUE_LIST, backpatching_child2, label_end[root]);
            break;  
        case ASSIGN_STMT:
            // 赋值语句只要弹出一个元素
            first_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            // 获取赋值变量
            result_str = root->token->val;
            push_list(MID_CODE_TYPE_ASSIGN, result_str, first_arg);
            break;
        case READ_STMT:
            // 直接读取对象
            result_str = root->token->val;
            push_list(MID_CODE_TYPE_READ, result_str);
            break;
        case WRITE_STMT:
            // 如果root的孩子不是factor，说明是表达式，弹出一个元素中间变量
            if (root->child[0]->node_type != FACTOR) {
                result_str = analyzing_stack.top().tk_val;
                analyzing_stack.pop();
            }
            // 如果root的孩子不是factor，那么直接读factor的变量名
            else {
                result_str = root->child[0]->token->val;
            }
            push_list(MID_CODE_TYPE_WRITE, result_str);
            break;
        case GREATER_THAN_EXPR:
            // 先输出label
            result_str = to_string(++label_num);
            push_list(MID_CODE_TYPE_LABEL, result_str);
            label_start[root] = label_num;

            // 从栈中弹出元素
            temp = ">" + analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val + temp;
            analyzing_stack.pop();

            result_str = "dest";
            backpatching_index = push_list(MID_CODE_TYPE_IF, result_str, first_arg);
            push_list(MID_CODE_TYPE_GOTO, "dest");
        
            // 进行拉链回填
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, backpatching_index);
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, backpatching_index + 1);
            break;
        case LESS_THAN_EXPR:
            // 先输出label
            result_str = to_string(++label_num);
            push_list(MID_CODE_TYPE_LABEL, result_str);
            label_start[root] = label_num;

            // 从栈中弹出元素
            temp = "<" + analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val + temp;
            analyzing_stack.pop();

            result_str = "dest";
            backpatching_index = push_list(MID_CODE_TYPE_IF, result_str, first_arg);
            push_list(MID_CODE_TYPE_GOTO, "dest");

            // 进行拉链回填
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, backpatching_index);
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, backpatching_index + 1);
            break;
        case GREATER_EQUAL_THAN_EXPR:
            // 先输出label
            result_str = to_string(++label_num);
            push_list(MID_CODE_TYPE_LABEL, result_str);
            label_start[root] = label_num;

            // 从栈中弹出元素
            temp = ">=" + analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val + temp;
            analyzing_stack.pop();

            result_str = "dest";
            backpatching_index = push_list(MID_CODE_TYPE_IF, result_str, first_arg);
            push_list(MID_CODE_TYPE_GOTO, "dest");

            // 进行拉链回填
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, backpatching_index);
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, backpatching_index + 1);
            break;
        case LESS_EQUAL_THAN_EXPR:
            // 先输出label
            result_str = to_string(++label_num);
            push_list(MID_CODE_TYPE_LABEL, result_str);
            label_start[root] = label_num;

            // 从栈中弹出元素
            temp = "<=" + analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val + temp;
            analyzing_stack.pop();

            result_str = "dest";
            backpatching_index = push_list(MID_CODE_TYPE_IF, result_str, first_arg);
            push_list(MID_CODE_TYPE_GOTO, "dest");

            // 进行拉链回填
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, backpatching_index);
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, backpatching_index + 1);
            break;
        case EQUAL_EXPR:
            // 先输出label
            result_str = to_string(++label_num);
            push_list(MID_CODE_TYPE_LABEL, result_str);
            // 输出label
            label_end[root] = label_num;

            // 从栈中弹出元素
            temp = "=" + analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val + temp;
            analyzing_stack.pop();

            result_str = "dest";
            backpatching_index = push_list(MID_CODE_TYPE_IF, result_str, first_arg);
            push_list(MID_CODE_TYPE_GOTO, "dest");

            // 进行拉链回填
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, backpatching_index);
            jump_ins_list.make_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, backpatching_index + 1);
            break;
        case AND_EXPR:
            // 获取回填儿子结点
            backpatching_child1 = root->child[0];
            backpatching_child2 = root->child[1];
            // 输出label
            label_start[root] = label_start[backpatching_child1];

            // 进行回填
            jump_ins_list.backpatching(JUMP_INS_TYPE_TRUE_LIST, backpatching_child1, label_start[backpatching_child2]);
            merge_list = jump_ins_list.merge_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, backpatching_child1, backpatching_child2);
            jump_ins_list.set_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, merge_list);
            jump_ins_list.set_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, jump_ins_list.get_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, backpatching_child2));
            break;
        case OR_EXPR:
            // 获取回填儿子结点
            backpatching_child1 = root->child[0];
            backpatching_child2 = root->child[1];
            label_start[root] = label_start[backpatching_child1];

            // 进行回填
            jump_ins_list.backpatching(JUMP_INS_TYPE_FALSE_LIST, backpatching_child1, label_start[backpatching_child2]);
            merge_list = jump_ins_list.merge_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, backpatching_child1, backpatching_child2);
            jump_ins_list.set_jump_ins_list(JUMP_INS_TYPE_TRUE_LIST, root, merge_list);
            jump_ins_list.set_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, root, jump_ins_list.get_jump_ins_list(JUMP_INS_TYPE_FALSE_LIST, backpatching_child2));
            break;
        case ADD_EXPR:
            // 弹出前两个元素
            second_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            // 获取t前缀的符号名
            result_str = "t" + to_string(t_num++);
            // 保存中间结果
            push_list(MID_CODE_TYPE_ADD, result_str, first_arg, second_arg);
            analyzing_stack.push(AnalyzingStackNode(root, result_str));
            break;
        case SUB_EXPR:
            // 弹出前两个元素
            second_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            // 获取t前缀的符号名
            result_str = "t" + to_string(t_num++);
            // 保存中间结果
            push_list(MID_CODE_TYPE_SUB, result_str, first_arg, second_arg);
            analyzing_stack.push(AnalyzingStackNode(root, result_str));
            break;
        case MUL_EXPR:
            // 弹出前两个元素
            second_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            // 获取t前缀的符号名
            result_str = "t" + to_string(t_num++);
            // 保存中间结果
            push_list(MID_CODE_TYPE_MUL, result_str, first_arg, second_arg);
            analyzing_stack.push(AnalyzingStackNode(root, result_str));
            break;
        case DIV_EXPR:
            // 弹出前两个元素
            second_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            first_arg = analyzing_stack.top().tk_val;
            analyzing_stack.pop();
            // 获取t前缀的符号名
            result_str = "t" + to_string(t_num++);
            // 保存中间结果
            push_list(MID_CODE_TYPE_DIV, result_str, first_arg, second_arg);
            analyzing_stack.push(AnalyzingStackNode(root, result_str));
            break;
        case FACTOR:
            if (root->token) {
                int token_type = root->token->type;
                switch (token_type) {
                    case ID:
                        analyzing_stack.push(AnalyzingStackNode(root, root->token->val));
                        break;
                    case NUM:
                        analyzing_stack.push(AnalyzingStackNode(root, root->token->val));
                        break;
                    case STRING:
                        analyzing_stack.push(AnalyzingStackNode(root, "\'" + root->token->val + "\'"));
                        break;
                }
            }
            break;
    }
}

// 生成并打印中间代码
void generate_and_print_middle_code(FILE *fp, bool is_optimize) {
    // 获取语法树的根结点
    SyntaxTreeNode *root = create_syntax_tree(fp);
    // 初始化label前缀和t前缀的序号
    int label_num = 0;
    int t_num = 0;
    // 生成中间代码
    generate_middle_code(root, label_num, t_num);
    // 根据选项决定是否要优化
    if (is_optimize) {
        jump_ins_list.optimize_middle_codes();
    }
    
    // 打印符号表
    symbol_table.print_symbol_table();

    // 打印三地址中间代码
    int size = jump_ins_list.middle_codes.size();
    for (int i = 0; i < size; ++i) {
        printf("(%d) %s\n", i + 1, jump_ins_list.middle_codes[i].get_middle_code_str().c_str());
    }
}