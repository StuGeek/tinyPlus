%{
#include "token.h"
int cur_line_num = 1;
void lex_error(char* msg, int line);
%}

/* Definitions */
KEY                 ("write"|"read"|"if"|"then"|"else"|"return"|"begin"|"end"|"main"|"string"|"int"|"real"|"or"|"and"|"int"|"bool"|"char"|"while"|"do"|"repeat"|"until")
SYM                 (";"|","|"("|")"|"+"|"-"|"*"|"/"|":="|"=="|"!="|">"|"<"|"<="|">="|"="|"'")
ID                  ([a-zA-z][a-zA-Z0-9]*)
NUM                 ([0-9][0-9]*)
STR                 (\'[^\'\n]*\')
UNTERM_STR1         ((\'[^\'\n]*))
UNTERM_STR2         (\'\n)
COMMENT             (\{[^\{\}]*\})
UNTERM_COMMENT      (\{[^\{\}]*[\{]*)
ILLEGAL_SYMBOL      ([^[a-zA-Z0-9]|";"|","|"("|")"|"+"|"-"|"*"|"/"|":="|"=="|"!="|">"|"<"|"<="|">="|"="|"'"])

%%

[\n]                { cur_line_num++;                       }
[ \t\r\a]+          { /* ignore all spaces */               }

{KEY}               { return T_KEY;             }
{SYM}               { return T_SYM;             }
{ID}                { return T_ID;              }
{NUM}               { return T_NUM;             }
{STR}               { return T_STR;             }
{COMMENT}           { /* skip for comment */    }  

{ILLEGAL_SYMBOL}    { lex_error("An illegal symbol was found", cur_line_num);                           }
{UNTERM_STR1}       { lex_error("The string is missing a closing quote", cur_line_num);                 }
{UNTERM_STR2}       { lex_error("The string is missing a closing quote", cur_line_num); cur_line_num++; }
{UNTERM_COMMENT}    { lex_error("The comment is missing a closing bracket", cur_line_num);              }

<<EOF>>             { return 0; }

%%

int main(int argc, char* argv[]) {
    int token;
    while (token = yylex()) {
        print_token(token);
        if (token == 260) {
            int i = 1;
            while (yytext[i] != '\'') {
                printf("%c", yytext[i]);
                i++;
            }
        }
        else {
            printf("%s", yytext);
        }
        printf(")\n");
    }
    return 0;
}

void lex_error(char* msg, int line) {
    printf("\nError at line %-3d: %s\n\n", line, msg);
}

int yywrap(void) {
    return 1;
}
