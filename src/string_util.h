#include <stdbool.h>
#include "tree/parser.h"

typedef struct
{
    char *string;
    size_t index;
} StringIterator;

bool is_space(char c);
bool is_digit(char c);
bool is_letter(char c);
bool is_opening_parenthesis(char *c);
bool is_closing_parenthesis(char *c);
bool is_delimiter(char *c);
bool begins_with(char *prefix, char *str);
size_t str_split(char *str, char **out_strs, size_t num_delimiters, ...);
char *perr_to_string(ParserError perr);
//void trim(char *str);

StringIterator get_iterator(char *string);
char get_next_char(StringIterator *iterator);
size_t console_strlen(char *str);
