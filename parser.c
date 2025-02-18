#include "ast.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

char * new_str(char * str, int len) {
    char * new_s = malloc(len+1);
    int i = 0;
    for (i = 0; i<len; i++) {
        new_s[i] = str[i];
    }
    new_s[len] = 0;
    return new_s;
}

token * new_token() {
    token *new_token = malloc(sizeof(token));
    new_token->composed_exp = 0;
    new_token->var = 0;
    return new_token;
}

pair * new_pair() {
    pair * new_pair = malloc(sizeof(pair));
    new_pair->token = new_token();
    new_pair->next = 0;
    return new_pair;
}

// 假设表达式的语法正确
char judge_type(char * exp) {
    char type = 0;
    if(exp[0] == '(') {
        type = COMPOSED;
    }else if(exp[0] == '"' || exp[0] == '\'') {
        type = STRING;
    }else {
        type = NUMBER;
        for (char * c = exp; *c; c++) {
            if (!(*c >= '0' && *c <= '9')) {
                type = VARIABLE;
            }
        }
    }
    return type;
}

// (mul (add (div a b) c) (div d e) f)
// 假设 exp 为 application，且在一行内
// 可直接求值的表达式给 eval 判断并求值，不用 parse
pair * parse(char * exp) {
    if(exp == 0) {
        return 0;
    }

    pair * parsed_exp = new_pair();
    pair * cur_exp    = parsed_exp;
    pair * pre_exp    = 0;

    int i = 0; // 当前字符索引
    int token_start_index = 0; // 当前 token 起始索引
    int end_i = 0; // 当前 token 结束索引
    int l_parenthesis = 0; // 当前括号层次
    char in_str = 0; // 当前token是否是字符串

    // 去除开头空格
    while (exp[i] == ' ')
        i++;
    if (exp[i] != '('){
        printf("[parse]: %s not a application\n", exp);
    }else {
        i++;
        l_parenthesis++;
    }

    while (exp[i] != 0 && exp[i] != '\n' && l_parenthesis >= 1) {
        if (l_parenthesis > 1) {
            if (exp[i] == '"' || exp[i] == '\'') {
                in_str = !in_str;
            }else if (exp[i] == '(' && !in_str) {
                l_parenthesis++;
            }else if (exp[i] == ')' && !in_str) {
                l_parenthesis--;
                if(l_parenthesis == 1) {
                    end_i = i;

                    if(cur_exp == 0) {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    cur_exp->token->composed_exp = parse(new_str(exp + token_start_index, end_i - token_start_index + 1));
                    cur_exp->token->type = COMPOSED;
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;

                    token_start_index = 0;
                }
            }
        }else if(in_str == 1) {
            if (exp[i] == '"' || exp[i] == '\'') {
                in_str = 0;

                end_i = i;
                if (cur_exp == 0) {
                    cur_exp = new_pair();
                    pre_exp->next = cur_exp;
                }
                cur_exp->token->value = malloc(sizeof(lisp_value));
                cur_exp->token->type = VALUE;
                cur_exp->token->value->str = new_str(exp + token_start_index, end_i - token_start_index + 1);
                cur_exp->token->value->type = STRING;
                pre_exp = cur_exp;
                cur_exp = cur_exp->next;

                token_start_index = 0;
            }
        }else if (l_parenthesis == 1 && !in_str) {
            if (exp[i] == '"' || exp[i] == '\'') {
                in_str = 1;

                if(token_start_index != 0) {
                    end_i = i - 1;
                    
                    if (cur_exp == 0) {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    char * t = new_str(exp + token_start_index, end_i - token_start_index + 1);
                    char type = judge_type(t);
                    if (type == NUMBER) {
                        cur_exp->token->type = VALUE;
                        cur_exp->token->value = malloc(sizeof(lisp_value));
                        cur_exp->token->value->num = atoi(t);
                    } else if (type == VARIABLE){
                        cur_exp->token->type = VARIABLE;
                        cur_exp->token->var = t;
                    }else {
                        printf("[parse]: exception when judge_type\n");
                        return 0;
                    }
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;
                }

                token_start_index = i;
            }else if (exp[i] == '(') {
                l_parenthesis++;

                if(token_start_index != 0) {
                    end_i = i - 1;
       
                    if (cur_exp == 0) {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    char * t = new_str(exp + token_start_index, end_i - token_start_index + 1);
                    char type = judge_type(t);
                    if (type == NUMBER) {
                        cur_exp->token->type = VALUE;
                        cur_exp->token->value = malloc(sizeof(lisp_value));
                        cur_exp->token->value->type = NUMBER;
                        cur_exp->token->value->num = atoi(t);
                    } else if (type == VARIABLE){
                        cur_exp->token->type = VARIABLE;
                        cur_exp->token->var = t;
                    }else {
                        printf("[parse]: exception when judge_type\n");
                        return 0;
                    }
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;
                }

                token_start_index = i;
            }else if (exp[i] == ' ' || exp[i] == ')') {
                if(token_start_index != 0) {
                    end_i = i - 1;

                    if (cur_exp == 0) {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    char * t = new_str(exp + token_start_index, end_i - token_start_index + 1);
                    char type = judge_type(t);
                    if (type == NUMBER) {
                        cur_exp->token->type = VALUE;
                        cur_exp->token->value = malloc(sizeof(lisp_value));
                        cur_exp->token->value->type = NUMBER;
                        cur_exp->token->value->num = atoi(t);
                    } else if (type == VARIABLE){
                        cur_exp->token->type = VARIABLE;
                        cur_exp->token->var = t;
                    }else {
                        printf("[parse]: exception when judge_type\n");
                        return 0;
                    }
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;

                    token_start_index = 0;
                }
                if(exp[i] == ')') {
                    l_parenthesis--;
                }
            }else if(token_start_index == 0) {
                token_start_index = i;
            }
        }
        i++;
    }

    // 去除末尾空格
    while (exp[i] == ' ') {
        i++;
    }

    // 退出时要么表达式已经闭合，要么遇到终止符
    // 理想的情况是，已闭合且遇到终止符
    // 表达式已经闭合且表达式字符串也终结的情况之外，定义为语法错误
    if (l_parenthesis != 0 || (exp[i] != 0 && exp[i] != '\n')) {
        printf("[parse]: %s\n", SYNTAX_ERROR);
        return 0;
    }

    return parsed_exp;
}

void show_parsed_exp(pair * exp) {
    if(exp == 0)
        return;
    putc('(', stdout);
    if (exp->token->type == COMPOSED) {
        show_parsed_exp(exp->token->composed_exp);
    }else if (exp->token->type == VALUE) {
        if(exp->token->value->type == NUMBER) {
            printf("%d ", exp->token->value->num);
        }else if(exp->token->value->type == STRING) {
            printf("%s ", exp->token->value->str);
        }
    }else if (exp->token->type == VARIABLE) {
        printf("%s ", exp->token->var);
    }
    show_parsed_exp( exp->next );
    putc(')', stdout);
}

token * car(pair * exp) {
    return exp->token;
}

pair * cdr(pair * exp) {
    return exp->next;
}

token * cadr(pair * exp) {
    if(exp->next) {
        return exp->next->token;
    }else {
        return 0;
    }
}

// int main(int argc, char const *argv[])
// {
//     char buffer[1024];
//     for (int i = 0; i < 1024; i++) {
//         buffer[i] = 0;
//     }
//     fgets(buffer, sizeof(buffer), stdin);
//     pair * parsed_exp = parse(buffer);
//     show_parsed_exp(parsed_exp);
//     puts("\n");
//     show_parsed_exp( car(cdr(parsed_exp))->composed_exp );
//     return 0;
// }
