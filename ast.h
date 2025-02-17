#ifndef AST_H
#define AST_H
#define VALUE        0
#define VARIABLE     1
#define NUMBER       2
#define STRING       3
#define SYMBOL       4
#define COMPOSED     5
#define UNDEFINED    6
#define PROCEDURE    7

typedef struct proc proc;
typedef struct token token;
typedef struct pair pair;

typedef struct lisp_value {
    int num;
    char *str;
    proc *proc;
    char type;
} lisp_value;

typedef struct token {
    char * var;
    lisp_value * value;
    pair * composed_exp;
    char type;
} token;

typedef struct pair {
    struct token * token;
    struct pair * next;
} pair;
#endif