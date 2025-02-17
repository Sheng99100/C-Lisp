#ifndef PROC_H
#define PROC_H
typedef struct env env;
typedef struct pair pair;
typedef struct lisp_value lisp_value;

#define PRIMITIVE_PROCEDURE 0
#define COMPOSED_PROCEDURE  1

typedef struct param_list {
    char * param;
    struct param_list *next;
} param_list;

typedef struct proc {
    char proc_type;
    param_list * params;
    // int params_count
    pair * body;
    lisp_value * (*primitive_proc) (int argc, lisp_value ** args);
    env * env;
} proc;

typedef struct lambda {
    // int params_count
    param_list * params;
    pair * body;
} lambda;
#endif