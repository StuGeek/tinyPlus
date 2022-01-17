#ifndef _LEXICAL_H_
#define _LEXICAL_H_

#include <iostream>
#include <string>

using namespace std;

// 当前行号
extern int cur_line_num;

// 有限状态机的状态集
enum FSM_STATE {
    STATE_START,             // 开始状态
    STATE_ID,                // 标识符状态
    STATE_NUM,               // 数字状态
    STATE_COMMENT,           // 注释状态
    STATE_ASSIGN,            // 赋值符号状态
    STATE_GREATER,           // 大于符号状态或大于等于符号状态
    STATE_LESS,              // 小于符号状态或小于等于符号状态
    STATE_STR,               // 字符串状态
    STATE_SUCCESS,           // 成功识别状态，结束
    STATE_FAILED             // 识别失败状态，出现词法错误，结束
};

// Token类型
enum TokenType {
    ID,                      // 标识符
    NUM,                     // 数字常量
    STRING,                  // 字符串常量

    // 关键字
    KEY_WRITE,               // write
    KEY_READ,                // read
    KEY_IF,                  // if
    KEY_THEN,                // then
    KEY_ELSE,                // else

    KEY_END,                 // end

    KEY_STRING,              // string
    KEY_INT,                 // int

    KEY_REPEAT,              // repeat
    KEY_UNTIL,               // until

    KEY_OR,                  // or
    KEY_AND,                 // and
    KEY_BOOL,                // bool
    KEY_WHILE,               // while
    KEY_DO,                  // do

    // 特殊符号
    SYM_GREATER_THAN,        // >
    SYM_LESS_THAN,           // <
    SYM_GREATER_EQUAL_THAN,  // >=
    SYM_LESS_EQUAL_THAN,     // <=  
    SYM_ASSIGN,              // :=
    SYM_EQUAL,               // =
    SYM_SEMICOLON,           // ;
    SYM_COMMA,               // ,
    SYM_LEFT_PARENTHESES,    // (
    SYM_RIGHT_PARENTHESES,   // )
    SYM_ADD,                 // +
    SYM_SUB,                 // -
    SYM_MUL,                 // *
    SYM_DIV,                 // /
    
    // 文件结束
    ENDOFFILE,
    // 出现错误
    ERROR
};

// Token数据结构
struct Token {
    TokenType type;  // token的类型
    string val;      // token的值

    Token() {}
    Token(TokenType type, string val): type(type), val(val) {}
};

// 获取下一个token
Token getNextToken(FILE *fp);
// 按格式打印token
void print_token(TokenType type, const char *cur_token_val);
// 打印所有token
void print_all_tokens(FILE *fp);

#endif