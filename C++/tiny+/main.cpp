#include <iostream>
#include <string.h>

#include "lexical.h"
#include "syntax.h"
#include "generation.h"

int main(int argc, char *argv[]) {

    // 如果没有输入文件名，进行提示
    if (argc < 2) {
        printf("Please input the file name!\n");
        exit(1);
    }

    // 打开相应文件
    char *filename = argv[1];
    FILE *fp;
    fopen_s(&fp, filename, "r");
    if (fp == NULL) {
        printf("File open error!\n");
        exit(1);
    }

    // 命令参数输入token，打印全部token序列
    if (argc == 3 && !strcmp(argv[2], "token")) {
        print_all_tokens(fp);
        exit(1);
    }

    // 命令参数输入tree，打印语法树
    if (argc == 3 && !strcmp(argv[2], "tree")) {
        // 生成语法树
        SyntaxTreeNode *root = create_syntax_tree(fp);
        // 打印语法树
        printf("Syntax tree:\n");
        print_syntax_tree(root);
        exit(1);
    }

    if (argc == 3 && !strcmp(argv[2], "optimize")) {
        // 对中间代码进行优化
        generate_and_print_middle_code(fp, true);
    }
    else {
        // 生成中间代码
        generate_and_print_middle_code(fp, false);
    }
}
