#include <stdbool.h>

#include "../parsing/node.h"
#include "../parsing/parser.h"

size_t ansi_strlen(char *str);
bool begins_with(char *prefix, char *str);
void trim(char *str);
char *perr_to_string(ParserError perr);
