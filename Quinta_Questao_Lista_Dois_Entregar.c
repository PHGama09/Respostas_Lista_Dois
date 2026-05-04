#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M 202
#define m 5

/* =========================================================
   CÓDIGO ORIGINAL DO ALUNO — NÃO MODIFICADO
   ========================================================= */

typedef struct operator
{
    int unability;
    int priority;
    char oprt;
} operator;

typedef struct node
{
    void *item;
    struct node* next;
} node;

typedef struct stack
{
    node *top;
} stack;

stack *create_stack()
{
    stack *new_stack = (stack *)malloc(sizeof(stack));
    new_stack->top = NULL;
    return new_stack;
}

int is_empty_but_exist(stack *stack)
{
    return (stack->top == NULL);
}

int is_empty(stack *stack)
{
    return (stack == NULL || stack->top == NULL);
}

void push(stack *stack, void *item)
{
    node *new_top = (node *)malloc(sizeof(node));
    if(new_top == NULL)
    {
        printf("Doenst have enough memory to allocate\n");
        return;
    }
    new_top->item = item;
    new_top->next = stack->top;
    stack->top = new_top;
}

void *pop(stack *stack)
{
    if(is_empty(stack))
    {
        printf("Stack underflow\n");
        return NULL;
    }
    node *old_top = stack->top;
    void *item = old_top->item;
    stack->top = old_top->next;
    free(old_top);
    return item;
}

void *peek(stack *stack)
{
    if(is_empty_but_exist(stack))
        return NULL;
    if(is_empty(stack))
    {
        printf("Stack underflow\n");
        return NULL;
    }
    return stack->top->item;
}

void free_stack(stack *stack, void (*clear)(void *))
{
    while(stack->top != NULL)
    {
        node *old_top = stack->top;
        stack->top = old_top->next;
        if(clear != NULL) (*clear)(old_top->item);
        free(old_top);
    }
    free(stack);
}

void print_operators(void *item)
{
    printf("%d %c\n", ((operator *)item)->priority, ((operator *)item)->oprt);
}

void print_stack(stack *stack, void (*print)(void *))
{
    node *top_at = stack->top;
    while(top_at != NULL)
    {
        if(top_at->item != NULL) (*print)(top_at->item);
        else printf("Doenst exist item in this node\n");
        top_at = top_at->next;
    }
}

int search(char opr, operator op[])
{
    int i = 0;
    while(op[i].oprt != opr && i < 5) i++;
    if(i == 5) { printf("Type a valid operator\n"); exit(1); }
    return i;
}

void reading(char sent_i[], char sent_f[], int *k, int *l, operator op[])
{
    int opr_num;
    stack *stack = create_stack();
    operator *item;
    int i = *k;
    int j = *l;
    while(sent_i[i] != '\0' && sent_i[i] != ')')
    {
        if(sent_i[i] == '(')
        {
            i += 1;
            reading(sent_i, sent_f, &i, &j, op);
        }
        else if(sent_i[i] >= 65 && sent_i[i] <= 90)
        {
            sent_f[j] = sent_i[i];
            j += 1;
        }
        else if(sent_i[i] != ')')
        {
            opr_num = search(sent_i[i], op);
            if(is_empty_but_exist(stack))
            {
                push(stack, &op[opr_num]);
            }
            else
            {
                item = peek(stack);
                if(item == NULL) { printf("Doenst have enough memory to allocate\n"); return; }
                if(item->unability == op[opr_num].unability && op[opr_num].unability == 1)
                {
                    while(item != NULL && item->priority > op[opr_num].priority)
                    {
                        item = pop(stack);
                        sent_f[j] = item->oprt;
                        j += 1;
                        item = peek(stack);
                    }
                }
                else
                {
                    while(item != NULL && item->priority >= op[opr_num].priority)
                    {
                        item = pop(stack);
                        sent_f[j] = item->oprt;
                        j += 1;
                        item = peek(stack);
                    }
                }
                push(stack, &op[opr_num]);
            }
        }
        i += 1;
    }
    item = peek(stack);
    while(item != NULL)
    {
        item = pop(stack);
        sent_f[j] = item->oprt;
        j += 1;
        item = peek(stack);
    }
    *k = i;
    *l = j;
    free_stack(stack, NULL);
}

/* =========================================================
   FIM DO CÓDIGO ORIGINAL
   ========================================================= */


/* =========================================================
   SEÇÃO 1 — AST (Árvore Sintática Abstrata)

   Cada nó da AST representa:
     - uma variável proposicional (folha): tipo = 'V', var = 'A'
     - um operador unário (NOT):          tipo = 'U', oprt = '!'
     - um operador binário:               tipo = 'B', oprt = '+','*','>','-'

   Filhos: left = operando único (NOT) ou operando esquerdo (binários)
           right = operando direito (binários), NULL para NOT
   ========================================================= */
typedef struct ASTNode
{
    char tipo;          /* 'V' = variável, 'U' = unário, 'B' = binário */
    char var;           /* letra da variável (só se tipo=='V')          */
    char oprt;          /* operador (só se tipo=='U' ou 'B')            */
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Aloca um nó folha (variável) */
ASTNode *ast_var(char v)
{
    ASTNode *n = malloc(sizeof(ASTNode));
    n->tipo = 'V'; n->var = v; n->oprt = 0;
    n->left = n->right = NULL;
    return n;
}

/* Aloca um nó de operador unário */
ASTNode *ast_unary(char op, ASTNode *child)
{
    ASTNode *n = malloc(sizeof(ASTNode));
    n->tipo = 'U'; n->oprt = op; n->var = 0;
    n->left = child; n->right = NULL;
    return n;
}

/* Aloca um nó de operador binário */
ASTNode *ast_binary(char op, ASTNode *l, ASTNode *r)
{
    ASTNode *n = malloc(sizeof(ASTNode));
    n->tipo = 'B'; n->oprt = op; n->var = 0;
    n->left = l; n->right = r;
    return n;
}

/* Copia profunda de uma AST (necessária para reescritas que duplicam subárvores) */
ASTNode *ast_copy(ASTNode *node)
{
    if(node == NULL) return NULL;
    if(node->tipo == 'V') return ast_var(node->var);
    if(node->tipo == 'U') return ast_unary(node->oprt, ast_copy(node->left));
    return ast_binary(node->oprt, ast_copy(node->left), ast_copy(node->right));
}

/* Libera toda a memória de uma AST */
void ast_free(ASTNode *node)
{
    if(node == NULL) return;
    ast_free(node->left);
    ast_free(node->right);
    free(node);
}

/* Imprime a AST em notação infixa com parênteses completos */
void ast_print(ASTNode *node)
{
    if(node == NULL) return;
    if(node->tipo == 'V') { printf("%c", node->var); return; }
    if(node->tipo == 'U') { printf("%c", node->oprt); ast_print(node->left); return; }
    /* binário */
    printf("(");
    ast_print(node->left);
    printf("%c", node->oprt);
    ast_print(node->right);
    printf(")");
}

/* =========================================================
   SEÇÃO 2 — CONSTRUÇÃO DA AST A PARTIR DA PÓS-FIXA

   Lê a string pós-fixa caractere a caractere:
     - variável → empilha nó folha
     - '!'      → desempilha 1 filho, empilha nó unário
     - binário  → desempilha 2 filhos (ordem importa!), empilha nó binário
   ========================================================= */
ASTNode *rpn_to_ast(char *rpn)
{
    /* pilha de ponteiros para ASTNode */
    ASTNode *stk[M];
    int top = -1;

    for(int i = 0; rpn[i] != '\0'; i++)
    {
        char c = rpn[i];
        if(c >= 'A' && c <= 'Z')
        {
            stk[++top] = ast_var(c);
        }
        else if(c == '!')
        {
            ASTNode *child = stk[top--];
            stk[++top] = ast_unary('!', child);
        }
        else /* binário: *, +, >, - */
        {
            ASTNode *r = stk[top--]; /* segundo operando (topo)     */
            ASTNode *l = stk[top--]; /* primeiro operando (abaixo)  */
            stk[++top] = ast_binary(c, l, r);
        }
    }
    return stk[top]; /* raiz da AST */
}

/* =========================================================
   SEÇÃO 3 — LOG DE PASSOS ALGÉBRICOS

   Cada reescrita aplicada é registrada numa lista encadeada
   para que possamos imprimir o histórico completo de passos.
   ========================================================= */
typedef struct StepLog
{
    char descricao[256];    /* nome da lei aplicada                  */
    char expressao[512];    /* expressão resultante após a reescrita */
    struct StepLog *next;
} StepLog;

/* Cabeça global da lista de passos */
static StepLog *log_head = NULL;
static StepLog *log_tail = NULL;

/* Adiciona um passo ao log */
// void log_step(const char *desc, ASTNode *node)
// {
//     StepLog *s = malloc(sizeof(StepLog));
//     strncpy(s->descricao, desc, 255);
//     s->descricao[255] = '\0';

//     /* captura a expressão atual como string usando buffer temporário */
//     /* redireciona stdout temporariamente para um buffer de string     */
//     /* como C puro não tem sprintf para AST, usamos uma função auxiliar */
//     s->expressao[0] = '\0';
//     s->next = NULL;

//     if(log_head == NULL) { log_head = log_tail = s; }
//     else { log_tail->next = s; log_tail = s; }
// }

/* Imprime e libera o log de passos */
void log_print_and_free()
{
    StepLog *cur = log_head;
    int step = 1;
    while(cur != NULL)
    {
        printf("  Passo %d [%s]\n", step++, cur->descricao);
        if(cur->expressao[0] != '\0')
            printf("    Resultado: %s\n", cur->expressao);
        StepLog *next = cur->next;
        free(cur);
        cur = next;
    }
    log_head = log_tail = NULL;
}

void log_clear()
{
    StepLog *cur = log_head;
    while(cur != NULL) { StepLog *next = cur->next; free(cur); cur = next; }
    log_head = log_tail = NULL;
}

/* Função auxiliar: escreve AST como string num buffer */
void ast_to_str(ASTNode *node, char *buf, int *pos, int maxlen)
{
    if(node == NULL || *pos >= maxlen - 1) return;
    if(node->tipo == 'V') { buf[(*pos)++] = node->var; return; }
    if(node->tipo == 'U')
    {
        buf[(*pos)++] = node->oprt;
        ast_to_str(node->left, buf, pos, maxlen);
        return;
    }
    buf[(*pos)++] = '(';

    if (node->tipo == 'B' && (node->oprt == '+' || node->oprt == '*')) {
        char sL[256] = {0}, sR[256] = {0};
        int pL = 0, pR = 0;
        ast_to_str(node->left, sL, &pL, 255);
        ast_to_str(node->right, sR, &pR, 255);
        if (strcmp(sL, sR) > 0) {
            ASTNode *tmp = node->left;
            node->left = node->right;
            node->right = tmp;
        }
    }
    
    ast_to_str(node->left,  buf, pos, maxlen);
    if(*pos < maxlen-1) buf[(*pos)++] = node->oprt;
    ast_to_str(node->right, buf, pos, maxlen);
    if(*pos < maxlen-1) buf[(*pos)++] = ')';
}

/* Versão que adiciona o passo ao log com a expressão formatada */
void log_step_with_expr(const char *desc, ASTNode *node)
{
    StepLog *s = malloc(sizeof(StepLog));
    strncpy(s->descricao, desc, 255);
    s->descricao[255] = '\0';

    int pos = 0;
    ast_to_str(node, s->expressao, &pos, 511);
    s->expressao[pos] = '\0';
    s->next = NULL;

    if(log_head == NULL) { log_head = log_tail = s; }
    else { log_tail->next = s; log_tail = s; }
}

/* =========================================================
   SEÇÃO 4 — REESCRITAS ALGÉBRICAS

   Cada função recebe uma AST e devolve uma nova AST
   com a lei aplicada em todos os nós onde for possível.
   A função é chamada recursivamente de baixo para cima
   (pós-ordem), garantindo que os nós filhos já estejam
   simplificados antes do nó pai.

   Leis implementadas:
     4.1 Eliminação do bicondicional:  A-B  →  (A>B)*(B>A)
     4.2 Eliminação da implicação:     A>B  →  !A+B
     4.3 Dupla negação:                !!A  →  A
     4.4 De Morgan (AND):              !(A*B) → !A+!B
     4.5 De Morgan (OR):               !(A+B) → !A*!B
     4.6 Distribuição OR sobre AND:    A+(B*C) → (A+B)*(A+C)
   ========================================================= */

/* 4.1 — Elimina bicondicional: A-B  →  (A>B)*(B>A) */
ASTNode *elim_bicond(ASTNode *node)
{
    if(node == NULL) return NULL;
    /* aplica nos filhos primeiro (pós-ordem) */
    node->left  = elim_bicond(node->left);
    node->right = elim_bicond(node->right);

    if(node->tipo == 'B' && node->oprt == '-')
    {
        /* cria (A>B)*(B>A) com cópias dos filhos */
        ASTNode *A  = node->left;
        ASTNode *B  = node->right;
        ASTNode *AB = ast_binary('>', A,              ast_copy(B));
        ASTNode *BA = ast_binary('>', ast_copy(B),    ast_copy(A));
        /* libera o nó original (filhos foram reutilizados/copiados) */
        free(node);
        return ast_binary('*', AB, BA);
    }
    return node;
}

/* 4.2 — Elimina implicação: A>B  →  !A+B */
ASTNode *elim_impl(ASTNode *node)
{
    if(node == NULL) return NULL;
    node->left  = elim_impl(node->left);
    node->right = elim_impl(node->right);

    if(node->tipo == 'B' && node->oprt == '>')
    {
        ASTNode *notA = ast_unary('!', node->left);
        ASTNode *B    = node->right;
        free(node);
        return ast_binary('+', notA, B);
    }
    return node;
}

/* 4.3 — Dupla negação: !!A  →  A  (aplicada repetidamente até estabilizar) */
ASTNode *elim_double_neg(ASTNode *node)
{
    if(node == NULL) return NULL;
    node->left  = elim_double_neg(node->left);
    node->right = elim_double_neg(node->right);

    if(node->tipo == 'U' && node->oprt == '!')
    {
        ASTNode *child = node->left;
        if(child != NULL && child->tipo == 'U' && child->oprt == '!')
        {
            /* !!A → A: retorna o neto, libera os dois nós de negação */
            ASTNode *grandchild = child->left;
            free(child);
            free(node);
            /* aplica recursivamente no resultado (pode ter nova dupla negação) */
            return elim_double_neg(grandchild);
        }
    }
    return node;
}

/* 4.4 e 4.5 — De Morgan:
     !(A*B) → !A+!B
     !(A+B) → !A*!B  */
ASTNode *de_morgan(ASTNode *node)
{
    if(node == NULL) return NULL;
    node->left  = de_morgan(node->left);
    node->right = de_morgan(node->right);

    if(node->tipo == 'U' && node->oprt == '!')
    {
        ASTNode *child = node->left;
        if(child != NULL && child->tipo == 'B' &&
           (child->oprt == '*' || child->oprt == '+'))
        {
            /* !(A op B) → (!A) op_inv (!B) */
            char new_op = (child->oprt == '*') ? '+' : '*';
            ASTNode *notL = ast_unary('!', child->left);
            ASTNode *notR = ast_unary('!', child->right);
            free(child);
            free(node);
            return ast_binary(new_op, notL, notR);
        }
    }
    return node;
}

/* 4.6 — Distribuição de OR sobre AND:
     A+(B*C)  →  (A+B)*(A+C)
     (B*C)+A  →  (B+A)*(C+A)  */
ASTNode *distribute_or(ASTNode *node)
{
    if(node == NULL) return NULL;
    node->left  = distribute_or(node->left);
    node->right = distribute_or(node->right);

    if(node->tipo == 'B' && node->oprt == '+')
    {
        ASTNode *l = node->left;
        ASTNode *r = node->right;

        /* caso: A + (B*C) */
        if(r != NULL && r->tipo == 'B' && r->oprt == '*')
        {
            ASTNode *A  = l;
            ASTNode *B  = r->left;
            ASTNode *C  = r->right;
            ASTNode *AB = ast_binary('+', ast_copy(A), B);
            ASTNode *AC = ast_binary('+', A,            C);
            free(r); free(node);
            return distribute_or(ast_binary('*', AB, AC));
        }
        /* caso: (B*C) + A */
        if(l != NULL && l->tipo == 'B' && l->oprt == '*')
        {
            ASTNode *B  = l->left;
            ASTNode *C  = l->right;
            ASTNode *A  = r;
            ASTNode *BA = ast_binary('+', B, ast_copy(A));
            ASTNode *CA = ast_binary('+', C, A);
            free(l); free(node);
            return distribute_or(ast_binary('*', BA, CA));
        }
    }
    return node;
}

/* =========================================================
   SEÇÃO 5 — PIPELINE DE CONVERSÃO PARA FNC VIA AST

   Aplica as reescritas na ordem correta e registra cada
   passo no log para exibição posterior.
   ========================================================= */
ASTNode *ast_to_fnc(ASTNode *node, int verbose)
{
    char buf[512]; int pos;

    /* Passo 0 — expressão original */
    if(verbose) log_step_with_expr("Expressao original", node);

    /* Passo 1 — elimina bicondicional */
    node = elim_bicond(node);
    if(verbose) log_step_with_expr("Eliminar bicondicional (A-B -> (A>B)*(B>A))", node);

    /* Passo 2 — elimina implicacao */
    node = elim_impl(node);
    if(verbose) log_step_with_expr("Eliminar implicacao (A>B -> !A+B)", node);

    /* Passo 3 — De Morgan + dupla negação (repetir até estabilizar) */
    int changed = 1;
    int iter = 0;
    while(changed && iter < 20)
    {
        pos = 0; ast_to_str(node, buf, &pos, 511); buf[pos] = '\0';
        char prev[512]; strcpy(prev, buf);

        node = de_morgan(node);
        node = elim_double_neg(node);

        pos = 0; ast_to_str(node, buf, &pos, 511); buf[pos] = '\0';
        changed = (strcmp(prev, buf) != 0);
        iter++;
    }
    if(verbose) log_step_with_expr("De Morgan + dupla negacao aplicados", node);

    /* Passo 4 — distribuição de OR sobre AND (repetir até estabilizar) */
    changed = 1; iter = 0;
    while(changed && iter < 30)
    {
        pos = 0; ast_to_str(node, buf, &pos, 511); buf[pos] = '\0';
        char prev[512]; strcpy(prev, buf);

        node = distribute_or(node);

        pos = 0; ast_to_str(node, buf, &pos, 511); buf[pos] = '\0';
        changed = (strcmp(prev, buf) != 0);
        iter++;
    }
    if(verbose) log_step_with_expr("Distribuicao OR sobre AND", node);

    return node;
}

/* =========================================================
   SEÇÃO 6 — NORMALIZAÇÃO DA AST EM FNC PARA COMPARAÇÃO

   Para comparar duas FNCs algebricamente, precisamos de uma
   representação canônica: uma string da AST em FNC serve,
   pois duas ASTs estruturalmente idênticas geram a mesma string.
   ========================================================= */
void ast_canonical_str(ASTNode *node, char *buf, int *pos, int maxlen)
{
    ast_to_str(node, buf, pos, maxlen);
}

/* =========================================================
   SEÇÃO 7 — EQUIVALÊNCIA ALGÉBRICA REAL  (i.2)

   Converte ambas as sentenças para FNC via reescritas,
   mostra os passos de cada uma e compara as formas canônicas.
   ========================================================= */
void check_equivalence_algebraic_real(char *rpn1, char *rpn2)
{
    printf("\n--- Conversao algebrica de S1 para FNC ---\n");
    ASTNode *ast1 = rpn_to_ast(rpn1);
    log_clear();
    ast1 = ast_to_fnc(ast1, 1);
    log_print_and_free();

    printf("\n--- Conversao algebrica de S2 para FNC ---\n");
    ASTNode *ast2 = rpn_to_ast(rpn2);
    log_clear();
    ast2 = ast_to_fnc(ast2, 1);
    log_print_and_free();

    /* compara as formas canônicas */
    char fnc1[512], fnc2[512];
    int p1 = 0, p2 = 0;
    ast_canonical_str(ast1, fnc1, &p1, 511); fnc1[p1] = '\0';
    ast_canonical_str(ast2, fnc2, &p2, 511); fnc2[p2] = '\0';

    printf("\nFNC de S1: %s\n", fnc1);
    printf("FNC de S2: %s\n", fnc2);
    printf("\nEquivalencia algebrica (via FNC): %s\n",
           strcmp(fnc1, fnc2) == 0 ? "EQUIVALENTES" : "NAO EQUIVALENTES");

    ast_free(ast1);
    ast_free(ast2);
}

/* =========================================================
   SEÇÃO 8 — FUNÇÕES DE TABELA VERDADE (mantidas do v1)
   ========================================================= */
int collect_vars(char *rpn, char vars[26])
{
    int count = 0;
    for(int i = 0; rpn[i] != '\0'; i++)
    {
        char c = rpn[i];
        if(c >= 'A' && c <= 'Z')
        {
            int found = 0;
            for(int j = 0; j < count; j++)
                if(vars[j] == c) { found = 1; break; }
            if(!found) vars[count++] = c;
        }
    }
    return count;
}

int eval_rpn(char *rpn, char vars[], int vals[], int nv)
{
    int stk[M]; int top = -1;
    for(int i = 0; rpn[i] != '\0'; i++)
    {
        char c = rpn[i];
        if(c >= 'A' && c <= 'Z')
        {
            for(int j = 0; j < nv; j++)
                if(vars[j] == c) { stk[++top] = vals[j]; break; }
        }
        else if(c == '!') { stk[top] = !stk[top]; }
        else if(c == '*') { int b=stk[top--]; int a=stk[top--]; stk[++top]=a&&b; }
        else if(c == '+') { int b=stk[top--]; int a=stk[top--]; stk[++top]=a||b; }
        else if(c == '>') { int b=stk[top--]; int a=stk[top--]; stk[++top]=(!a)||b; }
        else if(c == '-') { int b=stk[top--]; int a=stk[top--]; stk[++top]=(a==b); }
    }
    return stk[top];
}

int *truth_table(char *rpn, char vars[], int nv, int print_flag)
{
    int rows = 1 << nv;
    int *results = malloc(rows * sizeof(int));
    if(print_flag)
    {
        for(int j = 0; j < nv; j++) printf(" %c ", vars[j]);
        printf("| Result\n");
        for(int j = 0; j < nv*3+9; j++) printf("-");
        printf("\n");
    }
    for(int mask = 0; mask < rows; mask++)
    {
        int vals[26];
        for(int j = 0; j < nv; j++) vals[j] = (mask >> (nv-1-j)) & 1;
        results[mask] = eval_rpn(rpn, vars, vals, nv);
        if(print_flag)
        {
            for(int j = 0; j < nv; j++) printf(" %d ", vals[j]);
            printf("|   %d\n", results[mask]);
        }
    }
    return results;
}

void check_equivalence_table(char *rpn1, char *rpn2)
{
    char vars[26]; int nv = 0;
    nv = collect_vars(rpn1, vars);
    char vars2[26]; int nv2 = collect_vars(rpn2, vars2);
    for(int i = 0; i < nv2; i++)
    {
        int found = 0;
        for(int j = 0; j < nv; j++) if(vars[j]==vars2[i]){found=1;break;}
        if(!found) vars[nv++] = vars2[i];
    }
    int rows = 1 << nv;
    int *r1 = truth_table(rpn1, vars, nv, 0);
    int *r2 = truth_table(rpn2, vars, nv, 0);
    int eq = 1;
    for(int i = 0; i < rows; i++) if(r1[i]!=r2[i]){eq=0;break;}
    printf("Equivalencia por tabela verdade: %s\n", eq ? "EQUIVALENTES" : "NAO EQUIVALENTES");
    free(r1); free(r2);
}

void print_fnc(char *rpn)
{
    char vars[26]; int nv = collect_vars(rpn, vars);
    int rows = 1 << nv;
    int *results = truth_table(rpn, vars, nv, 0);
    printf("FNC (tabela verdade): ");
    int first_clause = 1;
    for(int mask = 0; mask < rows; mask++)
    {
        if(results[mask] == 0)
        {
            if(!first_clause) printf(" * ");
            printf("(");
            int first_lit = 1;
            for(int j = 0; j < nv; j++)
            {
                int bit = (mask >> (nv-1-j)) & 1;
                if(!first_lit) printf("+");
                if(bit==1) printf("!%c", vars[j]);
                else       printf("%c",  vars[j]);
                first_lit = 0;
            }
            printf(")");
            first_clause = 0;
        }
    }
    if(first_clause) printf("TRUE");
    printf("\n");
    free(results);
}

/* FNC algébrica: mostra os passos e a forma final */
void print_fnc_algebraic(char *rpn)
{
    printf("\n--- Passos da conversao algebrica para FNC ---\n");
    ASTNode *ast = rpn_to_ast(rpn);
    log_clear();
    ast = ast_to_fnc(ast, 1);
    log_print_and_free();
    printf("FNC algebrica final: ");
    ast_print(ast);
    printf("\n");
    ast_free(ast);
}

void print_fnd(char *rpn)
{
    char vars[26]; int nv = collect_vars(rpn, vars);
    int rows = 1 << nv;
    int *results = truth_table(rpn, vars, nv, 0);
    printf("FND: ");
    int first_term = 1;
    for(int mask = 0; mask < rows; mask++)
    {
        if(results[mask] == 1)
        {
            if(!first_term) printf(" + ");
            printf("(");
            int first_lit = 1;
            for(int j = 0; j < nv; j++)
            {
                int bit = (mask >> (nv-1-j)) & 1;
                if(!first_lit) printf("*");
                if(bit==0) printf("!%c", vars[j]);
                else       printf("%c",  vars[j]);
                first_lit = 0;
            }
            printf(")");
            first_term = 0;
        }
    }
    if(first_term) printf("FALSE");
    printf("\n");
    free(results);
}

void check_sat(char *rpn)
{
    char vars[26]; int nv = collect_vars(rpn, vars);
    int rows = 1 << nv;
    int *results = truth_table(rpn, vars, nv, 0);
    int sat = 0, sat_mask = -1;
    for(int mask = 0; mask < rows; mask++)
        if(results[mask]) { sat=1; sat_mask=mask; break; }
    if(sat)
    {
        printf("SAT: SATISFATIVEL\n");
        printf("  Exemplo de valoracao: ");
        for(int j = 0; j < nv; j++)
            printf("%c=%d ", vars[j], (sat_mask>>(nv-1-j))&1);
        printf("\n");
    }
    else printf("SAT: INSATISFATIVEL (contradicao)\n");
    free(results);
}

void check_negation(char *rpn1, char *rpn2)
{
    char vars[26]; int nv = 0;
    nv = collect_vars(rpn1, vars);
    char vars2[26]; int nv2 = collect_vars(rpn2, vars2);
    for(int i = 0; i < nv2; i++)
    {
        int found = 0;
        for(int j = 0; j < nv; j++) if(vars[j]==vars2[i]){found=1;break;}
        if(!found) vars[nv++] = vars2[i];
    }
    int rows = 1 << nv;
    int *r1 = truth_table(rpn1, vars, nv, 0);
    int *r2 = truth_table(rpn2, vars, nv, 0);
    int neg = 1;
    for(int i = 0; i < rows; i++) if(r1[i]==r2[i]){neg=0;break;}
    printf("Negacao: S1 %s negacao de S2\n", neg ? "E" : "NAO E");
    free(r1); free(r2);
}

void to_rpn(char *infix, char *rpn, operator op[])
{
    int k = 0, l = 0;
    memset(rpn, 0, M);
    reading(infix, rpn, &k, &l, op);
    rpn[l] = '\0';
}

/* =========================================================
   SEÇÃO 9 — MENU PRINCIPAL
   ========================================================= */
int main()
{
    operator op[5];
    op[0].oprt='!'; op[0].priority=5; op[0].unability=1;
    op[1].oprt='*'; op[1].priority=4; op[1].unability=2;
    op[2].oprt='+'; op[2].priority=3; op[2].unability=2;
    op[3].oprt='>'; op[3].priority=2; op[3].unability=2;
    op[4].oprt='-'; op[4].priority=1; op[4].unability=2;

    int opcao;
    char infix1[M], infix2[M], rpn1[M], rpn2[M];

    printf("=== Sistema de Logica Proposicional v2 ===\n");
    printf("Operadores: ! (NOT)  * (AND)  + (OR)  > (IMPLICA)  - (BICOND)\n");
    printf("Variaveis : letras maiusculas A-Z\n\n");
    printf("1 - Equivalencia por tabela verdade\n");
    printf("2 - Equivalencia algebrica (com passos das leis)\n"); // printf("2 - Equivalencia algebrica REAL (com passos das leis)\n");
    printf("3 - Gerar FNC (tabela verdade)\n");
    printf("4 - Gerar FNC algebrica (com passos)\n");
    printf("5 - Gerar FND\n");
    printf("6 - Verificar satisfatibilidade (SAT)\n");
    printf("7 - Verificar negacao\n");
    printf("8 - Mostrar tabela verdade\n");
    printf("Opcao: ");
    scanf("%d", &opcao); getchar();

    if(opcao == 1 || opcao == 2 || opcao == 7)
    {
        printf("Sentenca 1: "); fgets(infix1, M, stdin);
        infix1[strcspn(infix1,"\n")] = '\0';
        to_rpn(infix1, rpn1, op);

        printf("Sentenca 2: "); fgets(infix2, M, stdin);
        infix2[strcspn(infix2,"\n")] = '\0';
        to_rpn(infix2, rpn2, op);

        if     (opcao==1) check_equivalence_table(rpn1, rpn2);
        else if(opcao==2) check_equivalence_algebraic_real(rpn1, rpn2);
        else              check_negation(rpn1, rpn2);
    }
    else
    {
        printf("Sentenca: "); fgets(infix1, M, stdin);
        infix1[strcspn(infix1,"\n")] = '\0';
        to_rpn(infix1, rpn1, op);

        if     (opcao==3) print_fnc(rpn1);
        else if(opcao==4) print_fnc_algebraic(rpn1);
        else if(opcao==5) print_fnd(rpn1);
        else if(opcao==6) check_sat(rpn1);
        else if(opcao==8) { char vars[26]; int nv=collect_vars(rpn1,vars); truth_table(rpn1,vars,nv,1); }
        else printf("Opcao invalida.\n");
    }
    return 0;
}
