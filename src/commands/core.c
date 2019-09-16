#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "arith_context.h"
#include "arith_rules.h"
#include "core.h"
#include "console_util.h"
#include "evaluation.h"
#include "help.h"
#include "show_rules.h"
#include "load.h"
#include "show_tree.h"
#include "assignments.h"

#include "../engine/string_util.h"

#define INTERACTIVE_ASK_PREFIX "> "
#define COMMENT_PREFIX         "'"

struct Command
{
    void (*initHandler)();
    bool (*checkHandler)(char *input);
    void (*execHandler)(ParsingContext *ctx, char *input);
};

static ParsingContext *ctx;

// Debug command:

void debug_init()
{
    g_debug = false;
}

bool debug_check(char *input)
{
    return strcmp(input, "debug") == 0;
}

void debug_exec(__attribute__((unused)) ParsingContext *ctx, __attribute__((unused)) char *input)
{
    g_debug = !g_debug;
    whisper("debug %s\n", g_debug ? "on" : "off");
}

// Quit command:

void quit_init() { }

bool quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

void quit_exec(__attribute__((unused)) ParsingContext *ctx, __attribute__((unused)) char *input)
{
    exit(EXIT_SUCCESS);
}

/*
Evaluation is last command because its check function always returns true
*/
static const size_t NUM_COMMANDS = 9;
static const struct Command commands[] = {
    { quit_init, quit_check, quit_exec },
    { debug_init, debug_check, debug_exec},
    { help_init, help_check, help_exec},
    { definition_init, definition_check, definition_exec},
    { rule_init, rule_check, rule_exec},
    { show_rules_init, show_rules_check, show_rules_exec},
    { show_tree_init, show_tree_check, show_tree_exec },
    { load_init, load_check, load_exec},
    { evaluation_init, evaluation_check, evaluation_exec}
};

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_util();
    ctx = arith_init_ctx();
    arith_init_rules(ctx);

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
            commands[i].execHandler(ctx, input);
            return;
        }
    }
}
