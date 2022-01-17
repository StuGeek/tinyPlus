#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <string>

using namespace std;

enum ERROR_TYPE {
    // 词法错误
    ERROR_STRING_SINGLE_QUOTES_MISSING,   // 字符串的单引号有缺失
    ERROR_ILLEGAL_SYMBOL,                 // 非法符号
    ERROR_COMMENTS_LEFT_BRACE_MISSING,    // 注释缺少左大括号
    ERROR_COMMENTS_RIGHT_BRACE_MISSING,   // 注释缺少右大括号
    ERROR_COMMENTS_LEFT_BRACE_SURPLUS,    // 注释多了左大括号嵌套错误
    ERROR_LETTER_AFTER_NUMBER,            // 字母后面紧接着数字
    ERROR_ASSIGN_SYMBOL_MISSING,          // 赋值符号没有打全

    // 语法错误
    ERROR_SYNTAX,

    // 语义错误
    ERROR_IDENTIFIER_WITHOUT_DECLARATION,  // 一个标识符没有声明就使用
    ERROR_DECLARE_MORE_THEN_ONCE,          // 一个标识符被不止一次声明
    ERROR_COND_TYPE_NOT_BOOL,              // 条件表达式的类型不是bool类型
    ERROR_OPERATION_NOT_EQUAL_TYPE,        // 一个二元操作符的两个操作数类型不相等
    ERROR_ASSIGN_NOT_EQUAL_TYPE            // 赋值语句左右部类型不相等
};

struct {
    ERROR_TYPE error_code;
    string error_message;
} errors[13] = {
    {ERROR_STRING_SINGLE_QUOTES_MISSING,
     "Missing single quote for string!"},
    {ERROR_ILLEGAL_SYMBOL,
     "Found an illegal symbol!"},
    {ERROR_COMMENTS_LEFT_BRACE_MISSING,
     "The left brace is missing!"},
    {ERROR_COMMENTS_RIGHT_BRACE_MISSING,
     "The right brace is missing!"},
    {ERROR_COMMENTS_LEFT_BRACE_SURPLUS,
     "An nested comment is found!"},
    {ERROR_LETTER_AFTER_NUMBER,
     "Numbers cannot be followed by letters!"},
    {ERROR_ASSIGN_SYMBOL_MISSING,
     "The assignment symbols are not complete!"},
    {ERROR_SYNTAX,
     "There is a syntax error!"},
    {ERROR_IDENTIFIER_WITHOUT_DECLARATION,
	 "There is an identifier that is used without declaration"},
    {ERROR_DECLARE_MORE_THEN_ONCE,
     "One identifier can not be decalred more than once!"},
	{ERROR_COND_TYPE_NOT_BOOL,
	  "The type of the conditional expression is not bool!"},
	{ERROR_OPERATION_NOT_EQUAL_TYPE,
	 "Two operation number's types are not equal!"},
	{ERROR_ASSIGN_NOT_EQUAL_TYPE,
	 "The left and right types of assignment statement are not equal"}
};

#endif  //_ERRORS_H_