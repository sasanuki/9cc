#include "9cc.h"

void gen(Node *node){
    if(node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch(node->kind){
        case ND_ADD: printf("    add rax, rdi\n"); break;
        case ND_SUB: printf("    sub rax, rdi\n"); break;
        case ND_MUL: printf("    imul rax, rdi\n"); break;
        case ND_DIV: 
        // x86-64のレジスタ格納仕様の関係上、割り算のみややこしい形になっている
        // idivは符号有除算で、商をraxに余りをrdxにセットする
        // cqo命令を呼ぶと64bitから128bitに伸ばしてraxとrdxをセットする
        printf("    cqo\n");
        printf("    idiv rdi\n"); 
        break;
        case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
        case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
        case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
        case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}