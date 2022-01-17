#include <string.h>
#include <map>

#include "lexical.h"
#include "errors.h"

using namespace std;

#define BUFFER_MAX_LEN 1024 // 缓冲区最大长度

int cur_line_num = 0;  // 设置当前行号为0

// 关键字表，哈希表的key为关键字的单词，value为关键字的类型
map<string, TokenType> Keywords = {
    {"write", KEY_WRITE}, {"read", KEY_READ}, {"if", KEY_IF}, 
    {"then", KEY_THEN}, {"else", KEY_ELSE}, {"end", KEY_END},
    {"string", KEY_STRING}, {"int", KEY_INT}, {"repeat", KEY_REPEAT},
    {"until", KEY_UNTIL}, {"or", KEY_OR}, {"and", KEY_AND},
    {"bool", KEY_BOOL}, {"while", KEY_WHILE}, {"do", KEY_DO}
};

// 查找关键字类型
TokenType find_keyword_type(const string & key) {
    // 如果关键字表中存在查找的关键字，返回对应类型
    if (Keywords.count(key)) {
        return Keywords[key];
    }
    // 否则返回ID
    else {
        return ID;
    }
}

// 之前可能存在识别出的token，或者识别失败，就要返回上一个位置
void back_to_last_pos(bool isEOF, int & cur_pos) {
    if (!isEOF) {
        cur_pos--;
    }
}

// 获取下一个token
Token getNextToken(FILE *fp) {
    static char buffer[BUFFER_MAX_LEN];
    static int buffer_len = 0;           // 缓冲区的已读长度
    static int cur_pos = 0;              // 在当前行的读取字符位置
    static bool isEOF = false;           // 是否文件结束
    static int left_brace_num = 0;       // 保存左大括号的个数，表示嵌套数

    TokenType cur_token_type;            // 当前读取到的token的类型
    string cur_token_val;                // 当前读取到的token的值
    bool is_save_char;                   // 是否保存读取到的字符

    // 如果当前保存的左大括号数不为0.说明处在注释状态，否则处在开始状态
    FSM_STATE fsm_state;
    if (left_brace_num == 0) {
        fsm_state = STATE_START;
    }
    else {
        fsm_state = STATE_COMMENT;
    }

    // 如果识别还未结束
    while (fsm_state != STATE_SUCCESS && fsm_state != STATE_FAILED) {
        // 获取下一个字符
        char c;
        // 如果在当前行的读取字符位置大于等于缓冲区已读长度
        if (cur_pos >= buffer_len) {
            // 如果可以继续往缓冲区读取字符
            if (fgets(buffer, BUFFER_MAX_LEN - 1, fp)) {
                // 进行换行
                cur_line_num++;
                // 更新缓冲区的已读长度和当前行的读取字符位置
                buffer_len = strlen(buffer);
                cur_pos = 0;
                // 读取字符
                c = buffer[cur_pos++];
            }
            // 否则文件结束，读取的字符为EOF
            else {
                isEOF = true;
                c = EOF;
            }
        }
        // 否则直接读取字符
        else {
            c = buffer[cur_pos++];
        }

        is_save_char = true;
        switch (fsm_state) {
            // 初始状态
            case STATE_START:
                // 如果读取字符为字母，那么下一状态为标识符状态
                if (isalpha(c))
                    fsm_state = STATE_ID;
                // 如果读取字符为数字，那么下一状态为数字常量状态
                else if (isdigit(c))
                    fsm_state = STATE_NUM;
                // 如果读取字符为左大括号，那么下一状态为注释状态，不保存字符
                else if (c == '{') {
                    fsm_state = STATE_COMMENT;
                    is_save_char = false;
                }
                // 如果读取字符为右大括号，由于不是在注释状态下读到，那么下一状态为识别失败状态，不保存字符
                else if (c == '}') {
                    fsm_state = STATE_FAILED;
                    is_save_char = false;
                    cur_token_type = ERROR;
                    cur_token_val = errors[ERROR_COMMENTS_LEFT_BRACE_MISSING].error_message;
                }
                // 如果读取字符为:，那么下一状态为赋值符号状态
                else if (c == ':') {
                    fsm_state = STATE_ASSIGN;
                }
                // 如果读取字符为>，那么下一状态为大于等于符号状态
                else if (c == '>') {
                    fsm_state = STATE_GREATER;
                }
                // 如果读取字符为<，那么下一状态为小于等于符号状态
                else if (c == '<') {
                    fsm_state = STATE_LESS;
                }
                // 如果读取字符为'，那么下一状态为字符串状态
                else if (c == '\'') {
                    fsm_state = STATE_STR;
                }
                // 如果读取字符为空格、制表或回车，那么跳过这个字符，不保存
                else if (c == ' ' || c == '\t' || c == '\n') {
                    is_save_char = false;
                }
                // 如果读取到的是其它字符
                else {
                    fsm_state = STATE_SUCCESS;
                    switch (c) {
                        // 如果读到的是特殊符号，那么设置token的type为相应类型
                        case '=':
                            cur_token_type = SYM_EQUAL;
                            break;
                        case ';':
                            cur_token_type = SYM_SEMICOLON;
                            break;
                        case ',':
                            cur_token_type = SYM_COMMA;
                            break;
                        case '(':
                            cur_token_type = SYM_LEFT_PARENTHESES;
                            break;
                        case ')':
                            cur_token_type = SYM_RIGHT_PARENTHESES;
                            break;
                        case '+':
                            cur_token_type = SYM_ADD;
                            break;
                        case '-':
                            cur_token_type = SYM_SUB;
                            break;
                        case '*':
                            cur_token_type = SYM_MUL;
                            break;
                        case '/':
                            cur_token_type = SYM_DIV;
                            break;
                        // 如果读到文件结束符
                        case EOF:
                            // 不保存字符，且设置token类型为文件结束
                            is_save_char = false;
                            cur_token_type = ENDOFFILE;
                            break;
                        // 如果读到非法字符
                        default:
                            // 识别失败，不保存字符，设置token为错误类型
                            fsm_state = STATE_FAILED;
                            is_save_char = false;
                            cur_token_type = ERROR;
                            cur_token_val = errors[ERROR_ILLEGAL_SYMBOL].error_message + c;
                    }
                }
                break;
            // 注释状态
            case STATE_COMMENT:
                // 不保存字符，除左大括号、右大括号、文件结束符外的其它字符不处理
                is_save_char = false;
                // 如果读到文件结束符，那么说明注释没有右大括号，识别失败，设置错误类型
                if (c == EOF) {
                    fsm_state = STATE_FAILED;
                    cur_token_type = ERROR;
                    cur_token_val = errors[ERROR_COMMENTS_RIGHT_BRACE_MISSING].error_message;
                    back_to_last_pos(isEOF, cur_pos);
                }
                // 如果读取字符为{，因为处在注释状态，说明存在大括号嵌套，识别失败，设置错误类型
                else if (c == '{') {
                    left_brace_num++;
                    fsm_state = STATE_FAILED;
                    cur_token_type = ERROR;
                    cur_token_val = errors[ERROR_COMMENTS_LEFT_BRACE_SURPLUS].error_message;
                }
                // 如果读取字符为}，那么退出注释状态，下一状态为开始状态，设置保存的左大括号数为0
                else if (c == '}') {
                    fsm_state = STATE_START;
                    left_brace_num = 0;
                }
                break;
            // 数字状态
            case STATE_NUM:
                // 字母不能紧接数字，识别失败，设置错误类型
                if (isalpha(c)) {
                    fsm_state = STATE_FAILED;
                    is_save_char = false;
                    cur_token_type = ERROR;
                    cur_token_val = errors[ERROR_LETTER_AFTER_NUMBER].error_message;
                    back_to_last_pos(isEOF, cur_pos);
                }
                // 识别到其它字符，说明之前识别数字成功，设置token，回退一个位置再来识别
                else if (!isdigit(c)) {
                    fsm_state = STATE_SUCCESS;
                    is_save_char = false;
                    cur_token_type = NUM;
                    back_to_last_pos(isEOF, cur_pos);
                }
                break;
            // 标识符状态
            case STATE_ID:
                // 识别的字符不是数字或字母，说明之前识别标识符成功，设置token，回退一个位置再来识别
                if (!isdigit(c) && !isalpha(c)) {
                    fsm_state = STATE_SUCCESS;
                    is_save_char = false;
                    cur_token_type = ID;
                    back_to_last_pos(isEOF, cur_pos);
                }
                break;
            // 赋值符号状态
            case STATE_ASSIGN:
                // 之前识别的字符是:，之后识别的字符是=，那么识别成功，设置当前token
                if (c == '=') {
                    fsm_state = STATE_SUCCESS;
                    cur_token_type = SYM_ASSIGN;
                }
                // 之前识别的字符是:，之后识别的字符为其它，那么识别失败，设置错误类型
                else {
                    fsm_state = STATE_FAILED;
                    is_save_char = false;
                    cur_token_type = ERROR;
                    cur_token_val = errors[ERROR_ASSIGN_SYMBOL_MISSING].error_message;
                    back_to_last_pos(isEOF, cur_pos);
                }
                break;
            // 大于符号状态或大于等于符号状态
            case STATE_GREATER:
                fsm_state = STATE_SUCCESS;
                // 之前识别的字符为>，之后识别的字符为=，识别到token类型为大于等于
                if (c == '=') {
                    cur_token_type = SYM_GREATER_EQUAL_THAN;
                }
                // 之前识别的字符为>，之后识别的字符为其它，之前识别到的token类型为大于，回退一个位置再来识别
                else {
                    cur_token_type = SYM_GREATER_THAN;
                    is_save_char = false;
                    back_to_last_pos(isEOF, cur_pos);
                }
                break;
            // 小于符号状态或小于等于符号状态
            case STATE_LESS:
                fsm_state = STATE_SUCCESS;
                // 之前识别的字符为<，之后识别的字符为=，识别到token类型为小于等于
                if (c == '=') {
                    cur_token_type = SYM_LESS_EQUAL_THAN;
                }
                // 之前识别的字符为<，之后识别的字符为其它，之前识别到的token类型为小于，回退一个位置再来识别
                else {
                    cur_token_type = SYM_LESS_THAN;
                    is_save_char = false;
                    back_to_last_pos(isEOF, cur_pos);
                }
                break;
            // 字符串状态
            case STATE_STR:
                // 识别到右单引号，识别成功，设置token
                if (c == '\'') {
                    fsm_state = STATE_SUCCESS;
                    cur_token_type = STRING;
                }
                // 识别到换行或文件结束，字符串不完整，识别失败，设置错误类型
                else if (c == '\n' || c == EOF) {
                    fsm_state = STATE_FAILED;
                    cur_token_type = ERROR;
                    cur_token_val = errors[ERROR_STRING_SINGLE_QUOTES_MISSING].error_message;
                    back_to_last_pos(isEOF, cur_pos);
                }
            case STATE_SUCCESS:
            case STATE_FAILED:
                break;
        }
        // 如果识别到的字符要保存，那么拼接进当前token的值
        if (is_save_char) {
            cur_token_val += c;
        }
    }

    // 如果最后识别成功
    if (fsm_state == STATE_SUCCESS) {
        // 如果token的值在关键字表中存在，存储相应类型
        if (cur_token_type == ID) {
            cur_token_type = find_keyword_type(cur_token_val);
        }
        // 返回相应类型和值的token
        return Token(cur_token_type, cur_token_val);
    }
    // 识别失败返回错误类型
    else {
        return Token(ERROR, cur_token_val);
    }
}

// 按格式打印token
void print_token(TokenType type, const char *cur_token_val) {
    switch (type) {
        case ID:
            printf("(ID, %s)\n", cur_token_val);
            break;
        case NUM:
            printf("(NUM, %s)\n", cur_token_val);
            break;
        case STRING:
            printf("(STR, %s)\n", cur_token_val);
            break;
        case KEY_WRITE:  case KEY_READ:  case KEY_IF:
        case KEY_THEN:   case KEY_ELSE:  case KEY_END:
        case KEY_STRING: case KEY_INT:   case KEY_REPEAT:
        case KEY_UNTIL:  case KEY_OR:    case KEY_AND:
        case KEY_BOOL:   case KEY_WHILE: case KEY_DO:
            printf("(KEYWORD, %s)\n", cur_token_val);
            break;
        case SYM_GREATER_THAN:
            printf("(SYM_GREATER_THAN, >)\n");
            break;
        case SYM_LESS_THAN:
            printf("(SYM_LESS_THAN, <)\n");
            break;
        case SYM_GREATER_EQUAL_THAN:
            printf("(SYM_GREATER_EQUAL_THAN, >=)\n");
            break;
        case SYM_LESS_EQUAL_THAN:
            printf("(SYM_LESS_EQUAL_THAN, <=)\n");
            break;
        case SYM_ASSIGN:
            printf("(SYM_ASSIGN, :=)\n");
            break;
        case SYM_EQUAL:
            printf("(SYM_EQUAL, =)\n");
            break;
        case SYM_SEMICOLON:
            printf("(SYM_SEMICOLON, ;)\n");
            break;
        case SYM_COMMA:
            printf("(SYM_COMMA, ,)\n");
            break;
        case SYM_LEFT_PARENTHESES:
            printf("(SYM_LEFT_PARENTHESES, ()\n");
            break;
        case SYM_RIGHT_PARENTHESES:
            printf("(SYM_RIGHT_PARENTHESES, ))\n");
            break;
        case SYM_ADD:
            printf("(SYM_ADD, +)\n");
            break;
        case SYM_SUB:
            printf("(SYM_SUB, -)\n");
            break;
        case SYM_MUL:
            printf("(SYM_MUL, *)\n");
            break;
        case SYM_DIV:
            printf("(SYM_DIV, /)\n");
            break;
        case ERROR:
            printf("Found an error at line %d: %s\n", cur_line_num, cur_token_val);
            break;
        default:
            printf("Illegel token: %d\n", type);
        }
}

// 打印所有token
void print_all_tokens(FILE *fp) {
    while (true) {
        Token token = getNextToken(fp);
        if (token.type == ENDOFFILE) {
            break;
        }
        print_token(token.type, token.val.c_str());
    }
}