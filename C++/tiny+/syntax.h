#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include <iostream>
#include <map>
#include <string>

#include "lexical.h"

using namespace std;

// 树的结点类型
enum NodeType {
    PROGRAM,        // 程序
    DECLARATIONS,   // 声明
    STMT_SEQUENCE,  // 语句序列

    // statement语句
    WHILE_STMT,   // while语句
    IF_STMT,      // if语句
    REPEAT_STMT,  // repeat语句
    ASSIGN_STMT,  // assign语句
    READ_STMT,    // read语句
    WRITE_STMT,   // write语句

    // expression表达式
    GREATER_THAN_EXPR,        // 大于表达式
    LESS_THAN_EXPR,           // 小于表达式
    GREATER_EQUAL_THAN_EXPR,  // 大于等于表达式
    LESS_EQUAL_THAN_EXPR,     // 小于等于表达式
    EQUAL_EXPR,               // 等于表达式

    OR_EXPR,   // 或表达式
    AND_EXPR,  // 与表达式
    NOT_EXPR,  // 非表达式

    ADD_EXPR,  // 加法表达式
    SUB_EXPR,  // 减法表达式
    MUL_EXPR,  // 乘法表达式
    DIV_EXPR,  // 除法表达式
    FACTOR     // 因子
};

// 符号的对象类型
enum ObjectType {
    OBJTYPE_FUNC,  // 函数类型的对象
    OBJTYPE_VAR,   // 变量类型的对象
    OBJTYPE_CONST  // 常量类型的对象
};

// 符号的值的类型
enum ValueType {
    VALTYPE_INT,   // 整型类型的值
    VALTYPE_BOOL,  // 布尔类型的值
    VALTYPE_STR    // 字符串类型的值
};

// 符号数据结构
struct Symbol {
    Token *token;            // 符号的token
    ObjectType object_type;  // 符号的对象的类型
    ValueType value_type;    // 符号的值的类型
};

// 符号数据结构
struct SyntaxTreeNode {
    Token *token;              // 结点存储的token
    SyntaxTreeNode *child[3];  // 结点的孩子
    NodeType node_type;        // 结点类型
    ValueType value_type;      // 结点值的类型

    SyntaxTreeNode(NodeType node_type, ValueType value_type = VALTYPE_INT, Token *token = nullptr,
                   SyntaxTreeNode *c1 = nullptr, SyntaxTreeNode *c2 = nullptr, SyntaxTreeNode *c3 = nullptr)
        : node_type(node_type), value_type(value_type), token(token) {
        child[0] = c1;
        child[1] = c2;
        child[2] = c3;
    }

    static SyntaxTreeNode *
    create_node(NodeType node_type, SyntaxTreeNode *c1 = nullptr, SyntaxTreeNode *c2 = nullptr, SyntaxTreeNode *c3 = nullptr) {
        return new SyntaxTreeNode(node_type, VALTYPE_INT, nullptr, c1, c2, c3);
    }

    static SyntaxTreeNode *
    create_node(NodeType node_type, Token *token) {
        return new SyntaxTreeNode(node_type, VALTYPE_INT, token);
    }
};

// 符号表
struct SymbolTable {
    map<string, Symbol *> symbol_table;

    // 插入符号
    Symbol *insert(string &key) {
        Symbol *symbol = new Symbol();
        symbol_table[key] = symbol;
        return symbol;
    }

    // 查询符号
    Symbol *find(string &key) {
        if (symbol_table.count(key)) {
            return symbol_table[key];
        } else {
            return NULL;
        }
    }

    // 打印符号表
    void print_symbol_table() {
        printf("Variable | ObjectType | ValueType\n---------------------------------\n");
        for (auto &symbol : symbol_table) {
            printf("%-9s|", symbol.first.c_str());

            switch(symbol.second->object_type) {
                case OBJTYPE_FUNC:
                    printf("Func        |");
                    break;
                case OBJTYPE_VAR:
                    printf("Var         |");
                    break;
                case OBJTYPE_CONST:
                    printf("Const       |");
                    break;
                default:
                    break;
            }

            switch(symbol.second->value_type) {
                case VALTYPE_INT:
                    printf("Int\n");
                    break;
                case VALTYPE_BOOL:
                    printf("Bool\n");
                    break;
                case VALTYPE_STR:
                    printf("Str\n");
                    break;
                default:
                    break;
            }
        }
        printf("\n");
    }
};

extern SymbolTable symbol_table;

// 生成程序结点，program -> declarations stmt_sequence
SyntaxTreeNode *program(FILE *fp, Token &cur_token);

// 生成声明结点
// declarations -> decl;declarations | ε，
//   decl -> type-specifier varlist
//     type-specifier -> int | bool | char
//     varlist -> identifier { , identifier }
SyntaxTreeNode *declarations(FILE *fp, Token &cur_token);
// 生成语句序列结点
// stmt-sequence -> statement {; statement }
//   statement -> if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt | while-stmt
SyntaxTreeNode *stmt_sequence(FILE *fp, Token &cur_token);

// 生成while语句结点
SyntaxTreeNode *while_stmt(FILE *fp, Token &cur_token);
// 生成if语句结点
SyntaxTreeNode *if_stmt(FILE *fp, Token &cur_token);
// 生成repeat语句结点
SyntaxTreeNode *repeat_stmt(FILE *fp, Token &cur_token);
// 生成assign语句结点
SyntaxTreeNode *assign_stmt(FILE *fp, Token &cur_token, Token identifier_token);
// 生成read语句结点
SyntaxTreeNode *read_stmt(FILE *fp, Token &cur_token);
// 生成write语句结点
SyntaxTreeNode *write_stmt(FILE *fp, Token &cur_token);

// 生成比较逻辑操作表达式结点
SyntaxTreeNode *comparison_exp(FILE *fp, Token &cur_token);
// 生成或逻辑操作表达式结点
SyntaxTreeNode *or_exp(FILE *fp, Token &cur_token);
// 生成与逻辑操作表达式结点
SyntaxTreeNode *and_exp(FILE *fp, Token &cur_token);
// 生成加减计算操作表达式结点
SyntaxTreeNode *add_or_sub_exp(FILE *fp, Token &cur_token);
// 生成乘除计算操作表达式结点
SyntaxTreeNode *mul_or_div_exp(FILE *fp, Token &cur_token);
// 生成因子结点
SyntaxTreeNode *factor(FILE *fp, Token &cur_token);

// 生成语法树
SyntaxTreeNode *create_syntax_tree(FILE *fp);

// 打印语法树
void print_syntax_tree(SyntaxTreeNode *root);

#endif  //_SYNTAX_H_
