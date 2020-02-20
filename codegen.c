#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node){

    switch (node->kind) {
        case ND_NUM:
        printf("    push %d\n", node->val);
        break;

        case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov, [rax]\n");
        printf("    push rax\n");
        break;

        case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->lhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        break;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
        case '+':
        printf("    add rax, rdi\n");
        break;
        case '-':
        printf("    sub rax, rdi\n");
        break;
        case '*':
        printf("    imul rax, rdi\n");
        break;
        case '/':
        printf("    cqo\n");
        printf("    idiv rdi\n");
  }

    printf("    push rax\n");
}