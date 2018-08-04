#include <stdbool.h>

void init_commands();
void make_silent();
void main_interactive();
void parse_input(char *input);
void message(int prio, const char *format, ...);
