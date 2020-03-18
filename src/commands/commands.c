#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "cmd_evaluation.h"
#include "cmd_help.h"
#include "cmd_clear.h"
#include "cmd_load.h"
#include "cmd_debug.h"
#include "cmd_definition.h"
#include "cmd_table.h"
#include "../string_util.h"
#include "../console_util.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/simplification.h"

#define INTERACTIVE_ASK_PREFIX "> "
#define COMMENT_PREFIX         "'"

// Is set to true when a command reported an error, affects exit code
bool error;
// Is set to true when input comes from getline or readline
bool input_on_heap;

// Quit command:

bool cmd_quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

bool cmd_quit_exec(char *input)
{
    if (input_on_heap) free(input);
    exit(error ? EXIT_FAILURE : EXIT_SUCCESS);
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
    clear_composite_functions();
    unload_simplification();
    unload_console_util();
    unload_history();
}

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_core_ctx();
    init_simplification();
    init_console_util();
    init_history();
    error = false;
    input_on_heap = false;
}

/*
Summary: Loop to ask user or file for command, ignores comments
    You may want to call set_interactive before
Returns: True when no command exited with an error, false otherwise
*/
bool process_input(FILE *file)
{
    input_on_heap = true;
    char *input = NULL;
    while (ask_input(file, &input, INTERACTIVE_ASK_PREFIX))
    {
        if (!begins_with(COMMENT_PREFIX, input)) exec_command(input);
        free(input);
    }
    // Loop was exited because input was EOF
    if (g_interactive) printf("\n");
    return !error;
}

/*
Summary: Tries to apply a command to input
    First command whose check-function returns true is executed
*/
void exec_command(char *input)
{
    for (size_t i = 0; i < NUM_COMMANDS; i++)
    {
        if (commands[i].check_handler(input))
        {
            if (!commands[i].exec_handler(input))
            {
                error = true;
            }
            return;
        }
    }
}