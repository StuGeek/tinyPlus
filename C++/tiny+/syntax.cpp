#include <iostream>
#include <vector>
#include "syntax.h"
#include "lexical.h"
#include "errors.h"

using namespace std;

#define PER_TAB_SPACE_NUM 2  // 一次缩进的空格数

SymbolTable symbol_table;    // 符号表

// 复制token
Token *copy_token(Token & cur_token) {
    Token *temp_token = new Token();
    temp_token->val = cur_token.val;
    temp_token->type = cur_token.type;
    return temp_token;
}

// 打印语义错误
void print_sematic_error(const ERROR_TYPE &error_code, char *error_details = nullptr) {
    // 发现错误直接退出，不进行中间代码的生成
    printf("Found an error at line %d: %s\n", cur_line_num,
            errors[error_code].error_message.c_str());
    exit(0);
}

// 检查当前token的类型是否和想要检查的类型相匹配，匹配则获取下一个token作为当前token
bool check_and_get_next(FILE *fp, Token & cur_token, TokenType check_type) {
    // 如果一样，那么当前获取下一个token作为当前token
    if (cur_token.type == check_type) {
        Token token = getNextToken(fp);
        cur_token.type = token.type;
        cur_token.val = "";
        if (cur_token.type == NUM || cur_token.type == STRING || cur_token.type == ID) {
            cur_token.val = token.val;
        }
        return true;
    }
    // 否则报错
    else {
        printf("Found an error at line %d: %s\n", cur_line_num,
            errors[ERROR_SYNTAX].error_message.c_str());
        return false;
    }
}

// 检查当前token的类型是否和类型数组的任一类型相匹配，匹配则获取下一个token作为当前token
bool check_vector_and_get_next(FILE *fp, vector<TokenType> & types, Token & cur_token) {
    for (auto & type : types) {
        if (type == cur_token.type) {
            check_and_get_next(fp, cur_token, type);
            return true;
        }
    }
    return false;
}

// 检查当前token的类型是否和类型数组的任一类型相匹配
bool check_vector(vector<TokenType> & types, Token & cur_token) {
    for (auto & type : types)
        if (type == cur_token.type) {
            return true;
        }

    return false;
}

// 生成program结点，按照定义，program -> declarations stmt_sequence
SyntaxTreeNode *program(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *declarations_node = declarations(fp, cur_token);
    return stmt_sequence(fp, cur_token);
}

// 生成declarations结点
SyntaxTreeNode *declarations(FILE *fp, Token & cur_token) {
    while (cur_token.type == KEY_INT || cur_token.type == KEY_BOOL || cur_token.type == KEY_STRING) {
        Token temp_token = cur_token;
        do {
            // 跳过类型声明
            Token identifier = getNextToken(fp);
            cur_token = identifier;
            if (check_and_get_next(fp, cur_token, ID)) {
                // 如果一个标识符被声明不止一次，那么报错
                if (symbol_table.find(identifier.val)) {
                    print_sematic_error(ERROR_DECLARE_MORE_THEN_ONCE);
                }
                Symbol *symbol = symbol_table.insert(identifier.val);
                symbol->token = copy_token(identifier);
                symbol->object_type = OBJTYPE_VAR;
                switch (temp_token.type) {
                    case KEY_INT:
                        symbol->value_type = VALTYPE_INT;
                        break;
                    case KEY_BOOL:
                        symbol->value_type = VALTYPE_BOOL;
                        break;
                    case KEY_STRING:
                        symbol->value_type = VALTYPE_STR;
                        break;
                    default:
                        break;
                }
            }
        } while (cur_token.type == SYM_COMMA);
        check_and_get_next(fp, cur_token, SYM_SEMICOLON);
    }
    return NULL;
}

// 生成stmt_sequence结点
SyntaxTreeNode *stmt_sequence(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *node1 = nullptr, *node2 = nullptr;
    vector<TokenType> statement_type{KEY_IF, KEY_REPEAT, ID, KEY_READ, KEY_WRITE, KEY_WHILE};
    Token last_token = cur_token;

    while (check_vector_and_get_next(fp, statement_type, cur_token)) {
        switch (last_token.type) {
            case KEY_IF:
                // 构建if_stmt结点
                node2 = if_stmt(fp, cur_token);
                break;
            case KEY_REPEAT:
                // 构建repeat_stmt结点
                node2 = repeat_stmt(fp, cur_token);
                break;
            case ID:
                // 构建assign_stmt结点
                node2 = assign_stmt(fp, cur_token, last_token);
                break;
            case KEY_READ:
                // 构建read_stmt结点
                node2 = read_stmt(fp, cur_token);
                break;
            case KEY_WRITE:
                // 构建write_stmt结点
                node2 = write_stmt(fp, cur_token);
                break;
            case KEY_WHILE:
                // 构建while_stmt结点
                node2 = while_stmt(fp, cur_token);
                break;
            default:
                break;
        }
        if (node1 == nullptr) {
            node1 = node2;
        }
        else {
            node1 = SyntaxTreeNode::create_node(STMT_SEQUENCE, node1, node2);
        }
        if (cur_token.type == SYM_SEMICOLON) {
            check_and_get_next(fp, cur_token, SYM_SEMICOLON);
        }
        last_token = cur_token;
    }
    return node1;
}

// 生成while_stmt结点
SyntaxTreeNode *while_stmt(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *bool_exp = or_exp(fp, cur_token);
    SyntaxTreeNode *stmt_sequ = nullptr;

    // 如果条件表达式的类型不是bool类型，直接报错
    if (bool_exp->value_type != VALTYPE_BOOL) {
        print_sematic_error(ERROR_COND_TYPE_NOT_BOOL);
    }

    check_and_get_next(fp, cur_token, KEY_DO);
    stmt_sequ = stmt_sequence(fp, cur_token);
    check_and_get_next(fp, cur_token, KEY_END);

    return SyntaxTreeNode::create_node(WHILE_STMT, bool_exp, stmt_sequ);
}

// 生成if_stmt结点
SyntaxTreeNode *if_stmt(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *bool_exp = or_exp(fp, cur_token);

    // 如果条件表达式的类型不是bool类型，直接报错
    if (bool_exp->value_type != VALTYPE_BOOL) {
        print_sematic_error(ERROR_COND_TYPE_NOT_BOOL);
    }

    check_and_get_next(fp, cur_token, KEY_THEN);
    SyntaxTreeNode *then_stmt_sequ = stmt_sequence(fp, cur_token);
    SyntaxTreeNode *else_stmt_sequ = nullptr;
    if (cur_token.type == KEY_ELSE) {
        check_and_get_next(fp, cur_token, KEY_ELSE);
        else_stmt_sequ = stmt_sequence(fp, cur_token);
    }
    check_and_get_next(fp, cur_token, KEY_END);

    return SyntaxTreeNode::create_node(IF_STMT, bool_exp, then_stmt_sequ, else_stmt_sequ);
}

// 生成repeat_stmt结点
SyntaxTreeNode *repeat_stmt(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *repeat_stmt_sequ = stmt_sequence(fp, cur_token);
    check_and_get_next(fp, cur_token, KEY_UNTIL);
    SyntaxTreeNode *bool_exp = or_exp(fp, cur_token);
    return SyntaxTreeNode::create_node(REPEAT_STMT, repeat_stmt_sequ, bool_exp);
}

// 生成assign_stmt结点
SyntaxTreeNode *assign_stmt(FILE *fp, Token & cur_token, Token identifier_token) {
    string identifier_key = identifier_token.val;
    Symbol *identifier_symbol = symbol_table.find(identifier_key);

    // 一个标识符没有声明就使用
    if (identifier_symbol == nullptr) {
        print_sematic_error(ERROR_IDENTIFIER_WITHOUT_DECLARATION);
    }

    check_and_get_next(fp, cur_token, SYM_ASSIGN);
    SyntaxTreeNode *exp = or_exp(fp, cur_token);

    // 赋值语句左右部类型不相等
    if (identifier_symbol->value_type != exp->value_type) {
        print_sematic_error(ERROR_ASSIGN_NOT_EQUAL_TYPE);
    }

    SyntaxTreeNode *assign_node = SyntaxTreeNode::create_node(ASSIGN_STMT, exp);
    assign_node->token = identifier_symbol->token;

    return assign_node;
}

// 生成read_stmt结点
SyntaxTreeNode *read_stmt(FILE *fp, Token & cur_token) {
    Token identifier_token = cur_token;
    check_and_get_next(fp, cur_token, ID);
    SyntaxTreeNode *read_node = SyntaxTreeNode::create_node(READ_STMT);
    read_node->token = copy_token(identifier_token);
    return read_node;
}

// 生成write_stmt结点
SyntaxTreeNode *write_stmt(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *exp = or_exp(fp, cur_token);
    return SyntaxTreeNode::create_node(WRITE_STMT, exp);
}

// 生成comparison_exp结点
SyntaxTreeNode *comparison_exp(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *arithmetic_exp = add_or_sub_exp(fp, cur_token);
    SyntaxTreeNode *comparison_expr = nullptr;
    vector<TokenType> comp_op_list{SYM_GREATER_THAN, SYM_EQUAL, SYM_GREATER_EQUAL_THAN, SYM_LESS_EQUAL_THAN, SYM_LESS_THAN};
    NodeType type;
    if (check_vector(comp_op_list, cur_token)) {
        switch (cur_token.type) {
            case SYM_GREATER_THAN:
                type = GREATER_THAN_EXPR;
                break;
            case SYM_LESS_THAN:
                type = LESS_THAN_EXPR;
                break;
            case SYM_GREATER_EQUAL_THAN:
                type = GREATER_EQUAL_THAN_EXPR;
                break;
            case SYM_LESS_EQUAL_THAN:
                type = LESS_EQUAL_THAN_EXPR;
                break;
            case SYM_EQUAL:
                type = EQUAL_EXPR;
                break;
            default:
                break;
        }
        check_vector_and_get_next(fp, comp_op_list, cur_token);
        comparison_expr = comparison_exp(fp, cur_token);
    }
    if (!comparison_expr) {
        return arithmetic_exp;
    }
    SyntaxTreeNode *res_node = SyntaxTreeNode::create_node(type, arithmetic_exp, comparison_expr);
    
    if (comparison_expr) {
        // 一个二元操作符的两个操作数类型不相等
        if (comparison_expr->value_type != arithmetic_exp->value_type) {
            print_sematic_error(ERROR_OPERATION_NOT_EQUAL_TYPE);
        }
    }

    res_node->value_type = VALTYPE_BOOL;
    return res_node;
}

// 生成or_exp结点
SyntaxTreeNode *or_exp(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *and_expr = nullptr, *or_expr = nullptr;
    and_expr = and_exp(fp, cur_token);
    if (cur_token.type == KEY_OR) {
        check_and_get_next(fp, cur_token, KEY_OR);
        or_expr = or_exp(fp, cur_token);
    }
    else {
        return and_expr;
    }
    SyntaxTreeNode *res_node = SyntaxTreeNode::create_node(OR_EXPR, and_expr, or_expr);
    
    // 一个二元操作符的两个操作数类型不相等
    if (and_expr->value_type != VALTYPE_BOOL || or_expr->value_type != VALTYPE_BOOL) {
        print_sematic_error(ERROR_OPERATION_NOT_EQUAL_TYPE);
    }
    
    res_node->value_type = VALTYPE_BOOL;
    return res_node;
}

// 生成and_exp结点
SyntaxTreeNode *and_exp(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *and_expr = nullptr, *or_expr = nullptr;
    and_expr = comparison_exp(fp, cur_token);
    if (cur_token.type == KEY_AND) {
        check_and_get_next(fp, cur_token, KEY_AND);
        or_expr = and_exp(fp, cur_token);
    }
    else {
        return and_expr;
    }
    SyntaxTreeNode *res_node = SyntaxTreeNode::create_node(AND_EXPR, and_expr, or_expr);
    
    // 两个操作数类型不相等且不为bool类型
    if (and_expr->value_type != VALTYPE_BOOL || or_expr->value_type != VALTYPE_BOOL) {
        print_sematic_error(ERROR_OPERATION_NOT_EQUAL_TYPE);
    }
    
    res_node->value_type = VALTYPE_BOOL;
    return res_node;
}

// 生成add_or_sub结点
SyntaxTreeNode *add_or_sub_exp(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *mul_expr = nullptr, *add_expr = nullptr;
    mul_expr = mul_or_div_exp(fp, cur_token);
    vector<TokenType> add_op_list{SYM_ADD, SYM_SUB};
    NodeType type;
    if (check_vector(add_op_list, cur_token)) {
        switch (cur_token.type) {
            case SYM_ADD:
                type = ADD_EXPR;
                break;
            case SYM_SUB:
                type = SUB_EXPR;
                break;
            default:
                break;
        }
        check_vector_and_get_next(fp, add_op_list, cur_token);
        add_expr = add_or_sub_exp(fp, cur_token);
    }
    if (!add_expr) {
        return mul_expr;
    }

    SyntaxTreeNode *res_node = SyntaxTreeNode::create_node(type, mul_expr, add_expr);
    res_node->value_type = VALTYPE_INT;
    return res_node;
}

// 生成mul_or_div_exp结点
SyntaxTreeNode *mul_or_div_exp(FILE *fp, Token & cur_token) {
    SyntaxTreeNode *fac_node = nullptr, *mul_expr = nullptr;
    fac_node = factor(fp, cur_token);
    vector<TokenType> add_op_list{SYM_MUL, SYM_DIV};
    NodeType type;
    if (check_vector(add_op_list, cur_token)) {
        switch (cur_token.type) {
            case SYM_MUL:
                type = MUL_EXPR;
                break;
            case SYM_DIV:
                type = DIV_EXPR;
                break;
            default:
                break;
        }
        check_vector_and_get_next(fp, add_op_list, cur_token);
        mul_expr = mul_or_div_exp(fp, cur_token);
    }
    if (!mul_expr) {
        return fac_node;
    }

    SyntaxTreeNode *ret_node = SyntaxTreeNode::create_node(type, fac_node, mul_expr);
    ret_node->value_type = VALTYPE_INT;
    return ret_node;
}

// 生成factor结点
SyntaxTreeNode *factor(FILE *fp, Token & cur_token) {
    vector<TokenType> factor_first{ID, NUM, STRING, SYM_LEFT_PARENTHESES};
    
    SyntaxTreeNode *node1 = nullptr;
    if (cur_token.type == SYM_LEFT_PARENTHESES) {
        check_and_get_next(fp, cur_token, SYM_LEFT_PARENTHESES);
        SyntaxTreeNode *exp = and_exp(fp, cur_token);
        node1 = SyntaxTreeNode::create_node(FACTOR, exp);
        node1->value_type = exp->value_type;
        check_and_get_next(fp, cur_token, SYM_RIGHT_PARENTHESES);
    }
    else {
        node1 = SyntaxTreeNode::create_node(FACTOR, copy_token(cur_token));
        string identifier_key;

        switch (cur_token.type) {
            case STRING:
                node1->value_type = VALTYPE_STR;
                break;
            case NUM:
                node1->value_type = VALTYPE_INT;
                break;
            case ID:
                identifier_key = cur_token.val;
                Symbol *identifier_symbol;
                if ((identifier_symbol = symbol_table.find(identifier_key)) == nullptr) {
                    // 一个标识符没有声明就使用
                    print_sematic_error(ERROR_IDENTIFIER_WITHOUT_DECLARATION);
                    node1->value_type = VALTYPE_INT;
                }
                else
                    node1->value_type = identifier_symbol->value_type;
                break;
            default:
                node1 = nullptr;
        }
        check_vector_and_get_next(fp, factor_first, cur_token);
    }
    return node1;
}

// 创建语法树
SyntaxTreeNode *create_syntax_tree(FILE *fp) {
    // 获取第一个token
    Token token = getNextToken(fp);
    SyntaxTreeNode *root = program(fp, token);
    if (token.type != ENDOFFILE) {
        printf("Program exits halfway!\n");
    }
    return root;
}


// 打印相应空格数
void print_space_num(int space_num) {
    for (int i = 0; i < space_num; i++)
        printf(" ");
}

// 打印语法树
void print_syntax_tree(SyntaxTreeNode *root) {
    static int space_num = 0;  // 每一行前打印的空格数

    // 如果结点不是语句序列结点，那么进行缩进
    if (root->node_type != STMT_SEQUENCE) {
        space_num += PER_TAB_SPACE_NUM;
    }
    if (root) {
        print_space_num(space_num);
        switch (root->node_type) {
            case STMT_SEQUENCE:
                break;
            case WHILE_STMT:
                printf("STMT: (Key, While)\n");
                if (root->token)
                    printf("%s\n", root->token->val.c_str());
                break;
            case IF_STMT:
                printf("STMT: (Key, If)\n");
                if (root->token)
                    printf("%s\n", root->token->val.c_str());
                break;
            case REPEAT_STMT:
                printf("STMT: (Key, Repeat)\n");
                break;
            case ASSIGN_STMT:
                if (root->token)
                    printf("STMT: (Key, Assign to %s)\n", root->token->val.c_str());
                break;
            case READ_STMT:
                if (root->token)
                    printf("STMT: (Key, Read %s)\n", root->token->val.c_str());
                break;
            case WRITE_STMT:
                printf("STMT: (Key, Write)\n");
                break;
            case GREATER_THAN_EXPR:
                printf("EXP LogOp: (Symbol, >)\n");
                break;
            case LESS_THAN_EXPR:
                printf("EXP LogOp: (Symbol, <)\n");
                break;
            case GREATER_EQUAL_THAN_EXPR:
                printf("EXP LogOp: (Symbol, >=)\n");
                break;
            case LESS_EQUAL_THAN_EXPR:
                printf("EXP LogOp: (Symbol, <=)\n");
                break;
            case EQUAL_EXPR:
                printf("EXP LogOp: (Symbol, ==)\n");
                break;
            case OR_EXPR:
                printf("EXP LogOp: (Key, or)\n");
                break;
            case AND_EXPR:
                printf("EXP LogOp: (Key, and)\n");
                break;
            case NOT_EXPR:
                printf("EXP LogOp: (Key, not)\n");
                break;
            case ADD_EXPR:
                printf("EXP CalOp: (Symbol, +)\n");
                break;
            case SUB_EXPR:
                printf("EXP CalOp: (Symbol, -)\n");
                break;
            case MUL_EXPR:
                printf("EXP CalOp: (Symbol, *)\n");
                break;
            case DIV_EXPR:
                printf("EXP CalOp: (Symbol, /)\n");
                break;
            case FACTOR:
                if (root->token) {
                    TokenType type = root->token->type;
                    switch (type) {
                        case ID:
                            printf("ID: %s\n", root->token->val.c_str());
                            break;
                        case NUM:
                            printf("NUM: %s\n", root->token->val.c_str());
                            break;
                        case STRING:
                            printf("STR: \'%s\'\n", root->token->val.c_str());
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
                printf("Illegal node\n");
                break;
        }

        // 递归打印存在的孩子结点
        for (int i = 0; i < 3; i++) {
            if (root->child[i]) {
                print_syntax_tree(root->child[i]);
            }
        }
    }
    
    // 如果结点不是语句序列结点，打印结束返回时减去缩进
    if (root->node_type != STMT_SEQUENCE) {
        space_num -= PER_TAB_SPACE_NUM;
    }
}
