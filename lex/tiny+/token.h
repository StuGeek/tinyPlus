#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    T_KEY = 256, T_SYM, T_ID, T_NUM, T_STR
} TokenType;

static void print_token(int token) {
    static char* token_strs[] = {
        "KEY", "SYM", "ID", "NUM", "STR"
    };

    if (token < 256) {
        printf("%-20c", token);
    } else {
        printf("(%s, ", token_strs[token-256]);
    }
}

#endif
