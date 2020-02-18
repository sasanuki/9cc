#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,    // 記号
    TK_IDENT,       // 識別子
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
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_ASSIGN,  // =
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <, >
    ND_LE,      // <=, >=
    ND_LVAR,    // ローカル変数
    ND_NUM,     // 整数
}   NodeKind;

typedef struct Node Node;

// 抽象木構文のNode
struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺 left-hand side
    Node *rhs;      // 右辺 right-hand side
    int val;        // kindがND_NUMの場合のみ使う
    int offset;     // kindがND_LVARの場合のみ使う
};

// externは定義であって実装ではないので.cのどこかに実装しておく必要がある
// 現在着目しているトークン
extern Token *token;
// 入力プログラム
extern char *user_input;

// main
Token *tokenize(char *p);
Node *expr();
void gen(Node *node);

// parse
int expect_number();
void expect(char *op);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *expr();

// container
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
bool at_eof();
Node *new_node_num(int val);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswitch(char *p, char *q);
Token *tokenize(char *p);