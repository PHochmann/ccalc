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
#include "../string_util.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

#define INTERACTIVE_ASK_PREFIX "> "
#define COMMENT_PREFIX         "'"

// Quit command:

bool cmd_quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

bool cmd_quit_exec(char *input)
{
    free(input);
    exit(g_error ? EXIT_FAILURE : EXIT_SUCCESS);
    return true;
}

struct Command
{
    bool (*check_handler)(char*);
    bool (*exec_handler)(char*);
};

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
    init_console_util();
    arith_init_ctx();
    arith_init_rules();
    g_error = false;
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
        if (commands[i].check_handler(input))
        {
            if (!commands[i].exec_handler(input))
            {
                g_error = true;
            }
            return;
        }
    }
}
