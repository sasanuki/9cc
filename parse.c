#include "9cc.h"

// 関数プロト（Cの場合は使う関数を事前に分かっていないといけない）
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
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
