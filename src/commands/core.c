#include <stdio.h>
#include <string.h>

#include "core.h"
#include "cmd_evaluation.h"
#include "cmd_help.h"
#include "cmd_clear.h"
#include "cmd_load.h"
#include "cmd_debug.h"
#include "cmd_definition.h"
#include "cmd_table.h"
#include "console_util.h"
#include "table.h"
#include "../string_util.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

#define INTERACTIVE_ASK_PREFIX "> "
#define COMMENT_PREFIX         "'"

struct Command
{
    bool (*checkHandler)(char*);
    void (*execHandler)(char*);
};

bool cmd_debug_check(char *input);
void cmd_debug_exec(char *input);
bool cmd_quit_check(char *input);
void cmd_quit_exec(char *input);

static const size_t NUM_COMMANDS = 8;
static const struct Command commands[] = {
    { cmd_quit_check,       cmd_quit_exec },
    { cmd_help_check,       cmd_help_exec},
    { cmd_table_check,      cmd_table_exec },
    { cmd_definition_check, cmd_definition_exec},
    { cmd_clear_check,      cmd_clear_exec},
    { cmd_debug_check,      cmd_debug_exec },
    { cmd_load_check,       cmd_load_exec},
    /* Evaluation is last command. Its check function always returns true. */
    { cmd_evaluation_check, cmd_evaluation_exec}
};

/*
Summary: Frees all ressources so that heap is empty after exit, except data malloced by readline.
*/
void unload_commands()
{
    arith_unload_ctx();
    arith_unload_rules();
    unload_console_util();
}

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    atexit(unload_commands);
    init_console_util();
    arith_init_ctx();
    arith_init_rules();
}

/*
Summary: Loop to ask user or file for command, ignores comments
    You may want to call set_interactive before
*/
void process_input(FILE *file)
{
    char *input = NULL;

    while (ask_input(file, &input, INTERACTIVE_ASK_PREFIX))
    {
        if (!begins_with(COMMENT_PREFIX, input)) parse_command(input);
        free(input);
    }

    // Loop was exited because input was EOF
    if (g_interactive) printf("\n");
}

/*
Summary: Tries to apply a command to input
    First command whose check-function returns true is executed
*/
void parse_command(char *input)
{
    for (size_t i = 0; i < NUM_COMMANDS; i++)
    {
        if (commands[i].checkHandler(input))
        {
            commands[i].execHandler(input);
            return;
        }
    }
}

// Quit command:

bool cmd_quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

void cmd_quit_exec(char *input)
{
    free(input);
    exit(EXIT_SUCCESS);
}
