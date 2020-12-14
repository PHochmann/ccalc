#include <stdbool.h>

void init_argparse();
void unload_argparse();
void add_switch(char *long_str, char *alt_str, bool *detection);
bool *parse_arg(char *arg);
