#ifndef _GENERATION_H_
#define _GENERATION_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "syntax.h"

using namespace std;

// 中间代码类型
enum MiddleCodeType {
    MID_CODE_TYPE_READ,    // read
    MID_CODE_TYPE_WRITE,   // write
    MID_CODE_TYPE_LABEL,   // label
    MID_CODE_TYPE_IF,      // if
    MID_CODE_TYPE_GOTO,    // goto
    MID_CODE_TYPE_ASSIGN,  // assign
    MID_CODE_TYPE_ADD,     // add
    MID_CODE_TYPE_SUB,     // sub
    MID_CODE_TYPE_MUL,     // mul
    MID_CODE_TYPE_DIV      // div
};

// 中间代码结构体
struct MiddleCode {
    MiddleCodeType mcode_type;  // 中间代码类型
    string result;              // 唯一结果参数
    string first_arg;           // 第一个参数
    string second_arg;          // 第二个参数

    MiddleCode(MiddleCodeType mcode_type, const string &first_arg = "", 
                  const string &second_arg = "", const string &result = "") : 
                      mcode_type(mcode_type), first_arg(first_arg), second_arg(second_arg), result(result) {}

    // 获取中间代码的字符串
    string get_middle_code_str() {
        switch (mcode_type) {
            case MID_CODE_TYPE_READ:
                return "read " + result;
            case MID_CODE_TYPE_WRITE:
                return "write " + result;
            case MID_CODE_TYPE_LABEL:
                return "Label L" + result;
            case MID_CODE_TYPE_IF:
                return "if " + first_arg + " goto L" + result;
            case MID_CODE_TYPE_GOTO:
                return "goto L" + result;
            case MID_CODE_TYPE_ASSIGN:
                return result + ":=" + first_arg;
            case MID_CODE_TYPE_ADD:
                return result + ":=" + first_arg + "+" + second_arg;
            case MID_CODE_TYPE_SUB:
                return result + ":=" + first_arg + "-" + second_arg;
            case MID_CODE_TYPE_MUL:
                return result + ":=" + first_arg + "*" + second_arg;
            case MID_CODE_TYPE_DIV:
                return result + ":=" + first_arg + "/" + second_arg;
            default:
                return "";
        }
    }
};

// 分析栈中的结点
struct AnalyzingStackNode {
    SyntaxTreeNode *syntax_tree_node;  // 语法树结点
    string tk_val;                     // 对应的token的值

    AnalyzingStackNode(SyntaxTreeNode *syntax_tree_node, string tk_val) : syntax_tree_node(syntax_tree_node), tk_val(tk_val) {}
};

// 跳转指令列表类型
enum JumpInsListType {
    JUMP_INS_TYPE_TRUE_LIST,
    JUMP_INS_TYPE_FALSE_LIST,
    JUMP_INS_TYPE_NEXT_LIST
};

// 跳转指令列表
struct JumpInsList {
    vector<map<SyntaxTreeNode *, vector<int>>> jump_ins_list_map;
    vector<MiddleCode> middle_codes;

    JumpInsList() {
        jump_ins_list_map.resize(3);
    }

    // 设置列表
    void set_jump_ins_list(JumpInsListType list_type, SyntaxTreeNode *node, vector<int> list) {
        jump_ins_list_map[list_type][node] = list;
    }

    // 获取列表
    vector<int> get_jump_ins_list(JumpInsListType list_type, SyntaxTreeNode *node) {
        return jump_ins_list_map[list_type][node];
    }

    // 创建一个只包含i的列表
    void make_jump_ins_list(JumpInsListType list_type, SyntaxTreeNode *node, int i) {
        jump_ins_list_map[list_type][node] = vector<int>();
        jump_ins_list_map[list_type][node].push_back(i);
    }

    // 合并列表
    vector<int> merge_jump_ins_list(JumpInsListType list_type, SyntaxTreeNode *node1, SyntaxTreeNode *node2) {
        vector<int> list1 = jump_ins_list_map[list_type][node1];
        vector<int> list2 = jump_ins_list_map[list_type][node2];
        list1.insert(list1.end(), list2.begin(), list2.end());
        return list1;
    }

    // 回填操作
    void backpatching(JumpInsListType list_type, SyntaxTreeNode *node, int targetIndex) {
        for (auto &i : jump_ins_list_map[list_type][node]) {
            // 设置填的goto语句的终点
            middle_codes[i].result = to_string(targetIndex);
        }
    }

    // 中间代码优化
    void optimize_middle_codes() {
        map<string, int> label_goto;

        int size = middle_codes.size();
        // 删除没有被goto到的label
        for (int i = 0; i < size; i++) {
            if (middle_codes[i].mcode_type == MID_CODE_TYPE_IF || middle_codes[i].mcode_type == MID_CODE_TYPE_GOTO) {
                label_goto[middle_codes[i].result]++;
            }
        }

        // 倒序删除多余的goto语句
        for (int i = size - 1; i >= 0; i--) {
            if (middle_codes[i].mcode_type == MID_CODE_TYPE_LABEL) {
                if (label_goto.count(middle_codes[i].result) == 0) {
                    middle_codes.erase(middle_codes.begin() + i);
                }
            }
        }

        // 删除goto和同一个label紧接的语句块
        for (int i = size - 1; i >= 1; i--) {
            if (middle_codes[i].mcode_type == MID_CODE_TYPE_LABEL && middle_codes[i - 1].mcode_type == MID_CODE_TYPE_GOTO &&
                middle_codes[i].result == middle_codes[i - 1].result && label_goto[middle_codes[i].result] == 1) {
                middle_codes.erase(middle_codes.begin() + i);
                middle_codes.erase(middle_codes.begin() + i - 1);
                i--;
            }
        }
    }
};

// 生成并打印中间代码
void generate_and_print_middle_code(FILE *fp, bool is_optimize);

#endif //_GENERATION_H_
