#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "core.h"
#include "cmd_evaluation.h"
#include "cmd_help.h"
#include "cmd_clear.h"
#include "cmd_load.h"
#include "cmd_debug.h"
#include "cmd_definition.h"
#include "../console_util.h"

#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"
#include "../string_util.h"

#define INTERACTIVE_ASK_PREFIX "> "
#define COMMENT_PREFIX         "'"

struct Command
{
    void (*initHandler)();
    bool (*checkHandler)(char *input);
    void (*execHandler)(char *input);
};

void cmd_debug_init();
bool cmd_debug_check(char *input);
void cmd_debug_exec(char *input);
void cmd_quit_init();
bool cmd_quit_check(char *input);
void cmd_quit_exec(char *input);

static const size_t NUM_COMMANDS = 7;
static const struct Command commands[] = {
    { cmd_quit_init,       cmd_quit_check,       cmd_quit_exec },
    { cmd_help_init,       cmd_help_check,       cmd_help_exec},
    { cmd_definition_init, cmd_definition_check, cmd_definition_exec},
    { cmd_clear_init,      cmd_clear_check,      cmd_clear_exec},
    { cmd_debug_init,      cmd_debug_check,      cmd_debug_exec },
    { cmd_load_init,       cmd_load_check,       cmd_load_exec},
    /* Evaluation is last command. Its check function always returns true. */
    { cmd_evaluation_init, cmd_evaluation_check, cmd_evaluation_exec}
};

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_console_util();
    arith_init_ctx();
    arith_init_rules();

    for (size_t i = 0; i < NUM_COMMANDS; i++)
    {
        commands[i].initHandler();
    }
}

/*
Summary: Loop to ask user or file for command, ignores comments
    You may want to call set_interactive before
*/
void process_input(FILE *file)
{
    char *input = NULL;

    while (ask_input(INTERACTIVE_ASK_PREFIX, file, &input))
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

void cmd_quit_init() { }

bool cmd_quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

void cmd_quit_exec(__attribute__((unused)) char *input)
{
    exit(EXIT_SUCCESS);
}
