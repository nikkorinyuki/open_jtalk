typedef struct Label
{
    std::string string;
    std::string pos;
    std::string pos_group1;
    std::string pos_group2;
    std::string pos_group3;
    std::string ctype;
    std::string cform;
    std::string orig;
    std::string read;
    std::string pron;
    int acc;
    int mora_size;
    std::string chain_rule;
    int chain_flag;
} Label;