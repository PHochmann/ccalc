#include <stdbool.h>

bool tokenize(char *input, char **keywords, int num_keyw, char ***out_tokens, int *out_num_tokens);

bool is_space(char c);
bool is_letter(char c);
bool is_digit(char c);
bool is_opening_parenthesis(char c);
bool is_closing_parenthesis(char c);
bool is_delimiter(char c);
