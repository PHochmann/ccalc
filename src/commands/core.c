#include <stdio.h>
#include <string.h>

#include "core.h"
#include "util.h"
#include "command.h"
#include "evaluation.h"
#include "help.h"
#include "assignments.h"
#include "arith_context.h"

#define NUM_COMMANDS 6

static ParsingContext *ctx;
static Command commands[NUM_COMMANDS];

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_util();
    ctx = arith_get_ctx();

    commands[0] = get_command(quit_init, quit_check, quit_exec, "Quits the calculator");
    commands[1] = get_command(debug_init, debug_check, debug_exec, "Toggles debug mode");
    commands[2] = get_command(help_init, help_check, help_exec, "Displays help");
    commands[3] = get_command(definition_init, definition_check, definition_exec, "Defines new function");
    commands[4] = get_command(rule_init, rule_check, rule_exec, "Defines new rewrite rule");
    commands[5] = get_command(evaluation_init, evaluation_check, evaluation_exec, "Evaluates a mathematical expression");

    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        commands[i].initHandler();
    }
}

/*
Summary: Activates silent mode, whispered messages will not be displayed
*/
void make_silent()
{
    g_silent = true;
}

/*
Summary: Endless loop to ask user for command
*/
void main_interactive()
{
    char *input;

    while (true)
    {
        if (ask_input("> ", &input))
        {
            parse_command(input);
            free(input);
        }
        else return;
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