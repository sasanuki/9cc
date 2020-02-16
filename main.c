#include "9cc.h"

// 現在着目しているトークン
Token *token;
// 入力プログラム
char *user_input;

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木でコード生成
    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}