#include <stdio.h>
#include <string.h>

#include "core.h"
#include "util.h"
#include "command.h"
#include "evaluation.h"
#include "help.h"
#include "show_rules.h"
#include "load.h"
#include "assignments.h"
#include "arith_context.h"

#define NUM_COMMANDS 8

static ParsingContext *ctx;
static Command commands[NUM_COMMANDS];

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_util();
    ctx = arith_get_ctx();

    commands[0] = get_command(quit_init, quit_check, quit_exec);
    commands[1] = get_command(debug_init, debug_check, debug_exec);
    commands[2] = get_command(help_init, help_check, help_exec);
    commands[3] = get_command(definition_init, definition_check, definition_exec);
    commands[4] = get_command(rule_init, rule_check, rule_exec);
    commands[5] = get_command(show_rules_init, show_rules_check, show_rules_exec);
    commands[6] = get_command(load_init, load_check, load_exec);
    commands[7] = get_command(evaluation_init, evaluation_check, evaluation_exec);

    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        commands[i].initHandler();
    }
}

/*
Summary: Activates silent mode, whispered messages will not be displayed
*/
void make_interactive()
{
    g_interactive = true;
}

/*
Summary: Endless loop to ask user for command
*/
void process_input(FILE *file)
{
    char *input = NULL;

    while (true)
    {
        if (ask_input("> ", file, &input))
        {
            parse_command(input);
            free(input);
        }
        else
        {
            return;
        }
    }
}

void parse_command(char *input)
{
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        if (commands[i].checkHandler(input))
        {
            commands[i].execHandler(ctx, input);
            return;
        }
    }
}
