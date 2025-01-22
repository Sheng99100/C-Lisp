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

pair * new_pair() {
    pair * new_pair = malloc(sizeof(pair));
    new_pair->composed_exp = 0;
    new_pair->next = 0;
    new_pair->str = 0; 
    new_pair->var = 0;
    return new_pair;
}

// (mul (add (div a b) c) (div d e) f)
// 假设 exp 为 application，且在一行内
// 直接求值的表达式给 eval 判断
pair * parse(char * exp) {
    if(exp == 0) {
        return 0;
    }

    pair * parsed_exp = new_pair();
    pair * cur_exp    = parsed_exp;
    pair * pre_exp    = 0;

    int i = 0;
    int start_i = 0;
    int end_i = 0;
    int l_parenthesis = 0;
    char in_str = 0;

    while (exp[i] == ' ')
        i++;
    if (exp[i] != '(') {
        puts("not a application\n");
    }else {
        i++;
        l_parenthesis++;
    }
    
    while (exp[i] != 0 && l_parenthesis >= 1) {
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
                    cur_exp->composed_exp = parse(new_str(exp + start_i, end_i - start_i + 1));
                    cur_exp->type = COMPOSED;
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;

                    start_i = 0;
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
                cur_exp->str = new_str(exp + start_i, end_i - start_i + 1);
                cur_exp->type = STRING;
                pre_exp = cur_exp;
                cur_exp = cur_exp->next;

                start_i = 0;
            }
        }else if (l_parenthesis == 1 && !in_str) {
            if(exp[i] == '"' || exp[i] == '\'') {
                in_str = 1;

                if(start_i != 0) {
                    end_i = i - 1;
                    if (cur_exp == 0) {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    cur_exp->var = new_str(exp + start_i, end_i - start_i + 1);
                    cur_exp->type = SYMBOL;
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;
                }

                start_i = i;
            }else if (exp[i] == '(') {
                l_parenthesis++;

                if(start_i != 0) {
                    end_i = i - 1;
                    if (cur_exp == 0)
                    {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    cur_exp->var = new_str(exp + start_i, end_i - start_i + 1);
                    cur_exp->type = SYMBOL;
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;
                }

                start_i = i;
            }else if (exp[i] == ' ' || exp[i] == ')') {
                if(start_i != 0) {
                    end_i = i - 1;

                    if (cur_exp == 0)
                    {
                        cur_exp = new_pair();
                        pre_exp->next = cur_exp;
                    }
                    cur_exp->var = new_str(exp + start_i, end_i - start_i + 1);
                    cur_exp->type = SYMBOL;
                    pre_exp = cur_exp;
                    cur_exp = cur_exp->next;

                    start_i = 0;
                }
                if(exp[i] == ')') {
                    l_parenthesis--;
                }
            }else {
                if(start_i == 0) {
                    start_i = i;
                }
            }
        }
        i++;
    }

    // 退出时要么表达式已经闭合，要么遇到终止符
    // 在模型内，设计理想的情况是，已闭合且遇到终止符
    // 表达式已经闭合且表达式字符串也终结的情况之外，定义为语法错误
    if (l_parenthesis != 0 || (exp[i] != 0 && exp[i] != '\n')){
        fprintf(stderr, SYNTAX_ERROR);
        return 0;
    }

    return parsed_exp;
}

void show_parsed_exp(pair * exp) {
    if(exp == 0) return;

    putc('(', stdout);
    while (exp) {
        if(exp->type == COMPOSED) {
            show_parsed_exp(exp->composed_exp);
        }else if(exp->type == NUMBER) {
            printf("%d ", exp->num);
        }else if(exp->type == STRING){
            printf("%s ", exp->str);
        }else if (exp->type == SYMBOL){
            printf("%s ", exp->var);
        }
        exp = exp->next;
    }
    putc(')', stdout);
}

int main(int argc, char const *argv[])
{
    char buffer[1024];
    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
    }
    fgets(buffer, sizeof(buffer), stdin);
    pair * parsed_exp = parse(buffer);
    show_parsed_exp(parsed_exp);
    return 0;
}
