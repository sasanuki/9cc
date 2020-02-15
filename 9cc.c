#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,    // 記号
    TK_NUM,         // 整数トークン
    TK_EOF,         // 入力終端トークン
}   TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMのときの数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <, >
    ND_LE,  // <=, >=
    ND_NUM, // 整数
}   NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺 left-hand side
    Node *rhs;      // 右辺 right-hand side
    int val;        // kindがND_NUMの場合のみ使う
};

// 関数プロト（Cの場合は使う関数を事前に分かっていないといけない）
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 現在着目しているトークン
Token *token;
// 入力プログラム
char *user_input;

// エラー報告関数
// printfと同じ引数
void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときにはトークンを１つ読み進める
// それ以外の場合にはエラーを報告
void expect(char *op){
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len)){
        error_at(token->str, "'%C'ではありません", op);
    }
    token = token->next;
}

int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr(){
    Node *node = equality();

    for(;;){
        if(consume("+")){
            node = new_node(ND_ADD, node, equality());
        }
        else if (consume("-")) {
            node = new_node(ND_SUB, node, equality());
        }
        else{
            return node;
        }
    }
}

Node *equality(){
    Node *node = relational();

    for(;;){
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        }
        else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        }
        else{
            return node;
        }
    }
}

Node *relational(){
    Node *node = add();

    for(;;){
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        }
        else if(consume("<=")){
            node = new_node(ND_LE, node, add());
        }
        else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        }
        else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        }
        else{
            return node;
        }
    }
}

Node *add(){
    Node *node = mul();

    for(;;){
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        }
        else {
            return node;
        }
    }
}

Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume("*")){
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume("/")){
            node = new_node(ND_DIV, node, unary());
        }
        else{
            return node;
        }
    }
}

Node *unary(){
    if(consume("+")){
        return primary();
    }
    else if(consume("-")){
        // 0-x とすることで計算がおかしくならないようにしている
        return new_node(ND_SUB, new_node_num(0), primary());
    }

    return primary();
}

Node *primary(){
    // 「(」が入っていれば「)」があるはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    
    // そうでなければ数値
    return new_node_num(expect_number());
}

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

Token *new_token(TokenKind kind, Token *cur, char *str, int len){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswitch(char *p, char *q){
    // pとqを先頭nバイト分比較する
    return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p){
        // skip space char
        if (isspace(*p)){
            p++;
            continue;
        }

        if (startswitch(p, "==") || startswitch(p, "!=") ||
            startswitch(p, "<=") || startswitch(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

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