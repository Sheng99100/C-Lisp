#ifndef ENV_H
#define ENV_H
typedef struct proc proc;
typedef struct lisp_value lisp_value;

typedef struct var_val {
    char       * var;
    lisp_value * value;
    struct var_val * next;
} var_val;

typedef struct env {
    var_val * list;
    struct env * outer_env;
} env;
#endif
