typedef struct Label
{
    char *string;
    char *pos;
    char *pos_group1;
    char *pos_group2;
    char *pos_group3;
    char *ctype;
    char *cform;
    char *orig;
    char *read;
    char *pron;
    int acc;
    int mora_size;
    char *chain_rule;
    int chain_flag;
} Label;