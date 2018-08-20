#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "arith.h"
#include "commands.h"

#include "engine/parser.h"

#define VERSION "0.0.7"

void _exit()
{
	uninit_parser();
}

int main(int argc, char *argv[])
{
	atexit(_exit);

	init_parser();
	init_commands();
	
	if (argc > 1)
	{
		make_silent();
		for (int i = 1; i < argc; i++) parse_input(argv[i]);
	}
	else
	{
		printf("Calculator %s (c) 2018, Philipp Hochmann\n", VERSION);
		printf("Commands: help, def_func, def_rule\n");
		main_interactive();
	}
	
	return 0;
}
