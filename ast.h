#define NUMBER      0
#define STRING      1
#define SYMBOL      2
#define COMPOSED    3

typedef struct pair{
    int num;
    char * str;
    char * var;
    struct pair * composed_exp;

    struct pair * next;

    int type;
} pair;