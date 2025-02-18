#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error.h"
#include "ast.h"
#include "env.h"
#include "proc.h"
#include "parser.c"

lisp_value * set_var(char *var, lisp_value *val, env *env);
lisp_value * look_up_var(char * var, env * env);
lisp_value * eval(pair * exp, env * env);
void eval_define(pair * define_exp, env * env);
lisp_value * lisp_add(int argc, lisp_value ** args);
lisp_value * lisp_mul(int argc, lisp_value ** args);
lisp_value * lisp_sub(int argc, lisp_value ** args);
lisp_value * lisp_div(int argc, lisp_value ** args);
void init_global_env();


lambda * make_lambda(pair * define_exp) {
    pair * name_params = define_exp->next->token->composed_exp;
    pair * cur_param = name_params->next;

    param_list * lambda_params = malloc(sizeof(param_list));
    param_list * cur_lambda_param = lambda_params;
    while (cur_param) {
        cur_lambda_param->param = new_str(cur_param->token->var, strlen(cur_param->token->var));
        if (cur_param->next) {
            cur_lambda_param->next = malloc(sizeof(lambda));
            cur_lambda_param = cur_lambda_param->next;
        }else {
            cur_lambda_param->next = 0;
        }
        cur_param = cur_param->next;
    }

    lambda * new_lambda = malloc(sizeof(lambda));
    new_lambda->params = lambda_params;
    new_lambda->body   = define_exp->next->next->token->composed_exp;
    return new_lambda;
}

void eval_define(pair * define_exp, env * env) {
    if(define_exp->next == 0) {
        printf("[eval_define]: define nead a variable\n");
        return;
    }
    if(define_exp->next->next == 0) {
        printf("[eval_define]: define nead a value\n");
        return;
    }
    token * var = cadr(define_exp);
    
    if(var->type == VARIABLE) {
        token * val_token = define_exp->next->next->token;
        
        lisp_value * val = 0;
        if(val_token->type == VALUE) {
            val = val_token->value;
        }else if(val_token->type == VARIABLE) {
            val = look_up_var(val_token->var, env);
            if(val == 0) {
                printf("[eval_define]: variable %s is undefined\n", val_token->var);
                return;
            }
        }else if(val_token->type == COMPOSED) {
            val = eval(val_token->composed_exp, env);
            if (val == 0) {
                printf("[eval_define]: eval exception\n");
                return;
            }
        }
        set_var(var->var, val, env);
    }else if(var->type == COMPOSED) { // 定义函数
        pair * sign = var->composed_exp;
        if(sign->token->type != VARIABLE) {
            printf("proc name nead to be a variable\n");
        }else {
            lambda *new_lambda = make_lambda(define_exp);
            proc *new_proc = malloc(sizeof(proc));
            new_proc->params = new_lambda->params;
            new_proc->body = new_lambda->body;
            new_proc->proc_type = COMPOSED_PROCEDURE;
            new_proc->env = env;

            lisp_value * new_val = malloc(sizeof(lisp_value));
            new_val->type = PROCEDURE;
            new_val->proc = new_proc;

            set_var(sign->token->var, new_val, env);
        }
    }else {
        printf("[eval_define]: The variable part of define cannot be a value\n");
    }
}

lisp_value * basic_compute(int argc, lisp_value ** args, char oprator, int init_value) {
    if (argc < 0 || (argc > 0 && args == 0) || (argc == 0 && args != 0)) {
        printf("panic: [basic_compute]\n");
        exit(1);
    }
    int s = init_value;
    for (int i = 0; i < argc; i++) {
        if ( args[i]->type != NUMBER) {
            puts("type exception: this procedure expect number oprands\n");
            return 0;
        }else if(oprator == 0) {
            s += args[i]->num;
        }else if(oprator == 1) {
            s -= args[i]->num;
        }else if(oprator == 2) {
            s *= args[i]->num;
        }else if(oprator == 3) {
            s /= args[i]->num;
        }
    }
    lisp_value *res_val = malloc(sizeof(lisp_value));
    res_val->type = NUMBER;
    res_val->num = s;
    return res_val;
}

lisp_value * lisp_add(int argc, lisp_value ** args) {
    if (args == 0 && args == 0) {
        puts("exception: procedure [+] neads oprands\n");
        return 0;
    }
    return basic_compute(argc, args, 0, 0);
}

lisp_value * lisp_mul(int argc, lisp_value ** args) {
    if (args == 0 && args == 0) {
        puts("exception: procedure [*] neads oprands\n");
        return 0;
    }
    return basic_compute(argc, args, 2, 1);
}

lisp_value * lisp_sub(int argc, lisp_value ** args) {
    if (args == 0 && args == 0) {
        puts("type exception: procedure [-] neads oprands\n");
        return 0;
    }else if (argc < 0 || (argc > 0 && args == 0) || (argc == 0 && args != 0)){
        printf("panic: [basic_compute]\n");
        exit(1);
    }else if (args[0]->type != NUMBER) {
        puts("type exception: procedure [-] expect number oprands\n");
        return 0;
    }
    return basic_compute(argc, args, 1, argc > 1 ? args[0]->num : 0);
}

lisp_value * lisp_div(int argc, lisp_value ** args) {
    if (args == 0 && args == 0) {
        puts("type exception: procedure [/] neads oprands\n");
        return 0;
    }else if (argc < 0 || (argc > 0 && args == 0) || (argc == 0 && args != 0)) {
        printf("panic: [basic_compute]\n");
        exit(1);
    }else if (argc <= 0 || args == 0 || args[0]->type != NUMBER) {
        puts("type exception: procedure [/] expect number oprands\n");
        return 0;
    }
    return basic_compute(argc, args, 2, argc > 1 ? args[0]->num : 1);
}

env * global_env;

char * primitive_proc_names[] = {
    "+",
    "-",
    "*",
    "/"
};

lisp_value * (* primitive_proc_array[])(int argc, lisp_value ** args) = {
    lisp_add,
    lisp_sub,
    lisp_mul,
    lisp_div
};

void init_global_env() {
    global_env = malloc(sizeof(env));
    global_env->outer_env = 0;
    global_env->list = malloc(sizeof(var_val));
    var_val * cur_var_val = global_env->list;

    int proc_count = sizeof(primitive_proc_names) / sizeof(*primitive_proc_names);
    for (int i = 0; i < proc_count; i++) {
        cur_var_val->var = primitive_proc_names[i];

        cur_var_val->value = malloc(sizeof(lisp_value));

        cur_var_val->value->proc = malloc(sizeof(proc));
        cur_var_val->value->proc->primitive_proc = primitive_proc_array[i];
        cur_var_val->value->proc->proc_type = PRIMITIVE_PROCEDURE;
        cur_var_val->value->type = PROCEDURE;
        if(i+1 < proc_count) {
            cur_var_val->next = malloc(sizeof(var_val));
            cur_var_val = cur_var_val->next;
        }else {
            cur_var_val->next = 0;
        }
    }
}

lisp_value * look_up_var(char * var, env * env) {

    lisp_value * val = 0;
    var_val * var_val = 0;
    while (env != 0) {
        var_val = env->list;
        while (var_val != 0) {
            if(strcmp(var_val->var, var) == 0) {
                val = var_val->value;
                return val;
            }
            var_val = var_val->next;
        }
        env = env->outer_env;
    }
    return 0;
}

lisp_value * set_var(char *var, lisp_value *val, env *env) {
    var_val * var_val_list = env->list;
    var_val * cur_var_val = var_val_list;
    while (cur_var_val != 0) {
        if (strcmp(cur_var_val->var, var) == 0) {
            cur_var_val->value = val;
            return val;
        }
        if (cur_var_val->next == 0) {
            var_val * new_var_val = malloc(sizeof(var_val));
            new_var_val->next  = 0;
            new_var_val->var   = var;
            new_var_val->value = val;
            cur_var_val->next  = new_var_val;
            return val;
        }
        cur_var_val = cur_var_val->next;
    }
}

env * extend_env(env * base_env, var_val * var_val_list) {
    env * new_env = malloc(sizeof(env));
    new_env->outer_env = base_env;
    new_env->list = var_val_list;
    return new_env;
}

void show_lisp_value(lisp_value * val) {
    if( val->type == STRING ) {
        printf("%s\n", val->str);
    } else if (val->type == NUMBER) {
        printf("%d\n", val->num);
    } else if(val->type == PROCEDURE) {
        printf("[procedure]\n");
    }
}

lisp_value * eval(pair * exp, env * env) {
    lisp_value * res_val = 0;
    token * first_token = car(exp);
    if( first_token->type != VARIABLE ) { // 操作符部分必须是变量或 lambda
        if(first_token->value->type == NUMBER) {
            printf(" %d is not a function\n",  first_token->value->num );
        }else if (first_token->value->type == STRING) {
            printf(" %s is not a function\n", first_token->value->str);
        }
    }else { // 如果操作符部分是变量(包括define等)
        lisp_value * first_token_value = look_up_var(first_token->var, env);
        if( first_token_value == 0 ) { // 如果操作符部分的变量未定义
            if (strcmp(first_token->var, "define") == 0){
                eval_define(exp, env);
                return 0;
            }
            printf("[eval]: variable %s is undefined\n", first_token->var);
        }else if (first_token_value->type != PROCEDURE) {  // 如果操作符部分的变量未定义
            if (first_token_value->type == NUMBER) { // 如果操作符部分的变量是过程之外的值
                printf("[eval]: %d is not a function\n", first_token_value->num);
            }else if (first_token_value->type == STRING) {
                printf("[eval]: %s is not a function\n", first_token_value->str);
            }
        }else if(first_token_value->type == PROCEDURE) { // apply
            proc * proc = first_token_value->proc;
            
            if(proc->proc_type == PRIMITIVE_PROCEDURE) {
                int arg_count = 0; // 实参数量
                lisp_value ** args = malloc(sizeof(lisp_value *)); // 实参值数组
                pair * cur_arg = exp->next; // 当前实参表达式
                lisp_value * cur_val = 0; // 当前实参表达式的值
                while (cur_arg) {
                    if (cur_arg->token->type == VALUE)
                    { // 如果实参表达式时立即值
                        cur_val = cur_arg->token->value;
                    }
                    else if (cur_arg->token->type == VARIABLE)
                    { // 如果实参表达式是变量
                        cur_val = look_up_var(cur_arg->token->var, env);
                        if (cur_val == 0) {
                            printf("[eval]: variable %s is undefined\n", cur_arg->token->var);
                            return 0;
                        }
                    }
                    else if (cur_arg->token->type == COMPOSED)
                    { // 如果实参表达式是符合表达式
                        cur_val = eval(cur_arg->token->composed_exp, env);
                        if(cur_val == 0) {
                            printf("[eval]: eval exception\n");
                            return 0;
                        }
                    }
                    args[arg_count++] = cur_val;
                    if (cur_arg->next) {
                        args = realloc(args, sizeof(lisp_value *) * (arg_count + 1));
                    }
                    cur_arg = cur_arg->next;
                } 
                res_val = proc->primitive_proc(arg_count, args);
            }else if(proc->proc_type == COMPOSED_PROCEDURE) {
                var_val * var_val_list = malloc(sizeof(var_val)); // 准备调用环境的(变量-值对)列表
                param_list * cur_param = proc->params; // 形参名列表
                var_val * cur_var_val = var_val_list; // 当前变量值对
                pair * cur_arg = exp->next; // 当前实参表达式

                while (cur_arg)
                { // 遍历实参构建调用环境
                    if (!cur_param)
                    { // 有多余的 arg
                        printf("[eval]: too many args\n");
                        return 0;
                    }
                    cur_var_val->var = cur_param->param;
                    if (cur_arg->token->type == VALUE)
                    { // 如果实参表达式时立即值
                        cur_var_val->value = cur_arg->token->value;
                    }   
                    else if (cur_arg->token->type == VARIABLE)
                    { // 如果实参表达式是变量
                        cur_var_val->value = look_up_var(cur_arg->token->var, env);
                        if(cur_var_val->value == 0) {
                            printf("[eval]: variable %s is undefined\n", cur_arg->token->var);
                            return 0;
                        }
                    }
                    else if (cur_arg->token->type == COMPOSED)
                    { // 如果实参表达式是复合表达式
                        cur_var_val->value = eval(cur_arg->token->composed_exp, env);
                        if (cur_var_val->value == 0) {
                            printf("[eval]: eval exception\n");
                            return 0;
                        }
                    }

                    if(cur_arg->next){
                        cur_var_val->next = malloc(sizeof(var_val));
                        cur_var_val = cur_var_val->next;
                    }else {
                        cur_var_val->next = 0;
                    }

                    cur_param = cur_param->next;
                    cur_arg = cur_arg->next;
                }

                if (cur_param) { // 有 param 未满足 (缺少 arg)
                    printf("[eval]: The number of parameters may not match, cur_param = %s\n", cur_param->param);
                    return 0;
                }

                pair * body = proc->body;
                res_val = eval(body, extend_env(env, var_val_list));
            }
        }
    }

    return res_val;
}

void interpret(char * exp) {
    char type = judge_type(exp);
    if (type == STRING || type == NUMBER) { // 判断表达式类型
        printf(exp);
    }else if (type == VARIABLE){ // 变量则在全局环境求值
        lisp_value * res = look_up_var(exp, global_env);
        if( res != 0 ) {
            show_lisp_value(res);
        }else {
            printf("variable %s is undefined\n", exp);
        }
    }else if(type == COMPOSED) {
        pair * parsed_exp = parse(exp); 
        if(parsed_exp == 0) {
            printf("[interpret]: parse exception\n");
            return;
        }
        lisp_value * res  = eval(parsed_exp, global_env);
        if(res != 0) {
            show_lisp_value(res);
        }
    }
}

void main(int argc, char const *argv[]) {
    init_global_env();
    char buffer[1024];

    while (1){
        for (size_t i = 0; i < 1024; i++) {
            buffer[i] = 0;
        }

        printf("c-lisp>");

        fgets(buffer, sizeof(buffer), stdin);

        // 将第一个非空白字符的位置作为表达式起始地址
        char * exp = buffer;
        while (*exp == ' ') {
            exp++;
        }

        // fgets 会将换行符也读入，这里去掉换行符
        size_t len = strlen(exp);
        if (len > 0 && exp[len - 1] == '\n') {
            exp[len - 1] = '\0';
        }

        if (strcmp(exp, "exit") == 0) {
            printf("bye\n");
            exit(0);
        }

        interpret(exp);
    }
}