#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "cmd_evaluation.h"
#include "cmd_help.h"
#include "cmd_clear.h"
#include "cmd_load.h"
#include "cmd_definition.h"
#include "cmd_table.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/simplification.h"
#include "../parsing/tokenizer.h"

#include "../tree/tree_to_string.h"

#define INTERACTIVE_ASK_PREFIX "> "
#define COMMENT_PREFIX         '\''

// Is set to true when a command reported an error, affects exit code
bool error;

// Quit command:

int cmd_quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

bool cmd_quit_exec(__attribute__((unused)) char *input, __attribute__((unused)) int check_code)
{
    exit(error ? EXIT_FAILURE : EXIT_SUCCESS);
    return true;
}

struct Command
{
    /*
    Summary: Checks if passed input string is applicable to a command.
    Returns: 0 if command is not applicable, command-specific code otherwise.
    */
    int (*check_handler)(char* input);
    /*
    Summary: Exec handler is called when check handler returned a value other than 0.
    Params:
        input:      Input string to execute
        check_code: Return value of check handler
    Returns: True if command succeeded, False otherwise (to affect exit code)
    */
    bool (*exec_handler)(char* input, int check_code);
};

static const size_t NUM_COMMANDS = 7;
static const struct Command commands[] = {
    { cmd_quit_check,       cmd_quit_exec },
    { cmd_help_check,       cmd_help_exec },
    { cmd_table_check,      cmd_table_exec },
    { cmd_definition_check, cmd_definition_exec },
    { cmd_clear_check,      cmd_clear_exec },
    { cmd_load_check,       cmd_load_exec },
    /* Evaluation is last command. Its check function always returns true. */
    { cmd_evaluation_check, cmd_evaluation_exec }
};

/*
Summary: Frees all resources so that heap is empty after exit,
    except data malloced by readline and input of quit-command
*/
void unload_commands()
{
    clear_composite_functions();
    unload_simplification();
    unload_console_util();
    unload_history();
    unload_tokenizer();
}

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_core_ctx();
    init_tokenizer(g_ctx);
    init_simplification();
    init_console_util();
    init_history();
    error = false;
}

/*
Summary: Loop to ask user or file for command, ignores comments
    You may want to call set_interactive before.
Returns: True when no command exited with an error, false otherwise
*/
bool process_input(FILE *file)
{
    char *input = NULL;
    while (ask_input(file, &input, INTERACTIVE_ASK_PREFIX))
    {
        if (input[0] != '\0' && input[0] != COMMENT_PREFIX)
        {
            exec_command(input);
        }
        free(input);
    }

    if (g_interactive)
    {
        // Loop was exited because input was EOF
        printf("\n");
    }
    return !error;
}

/*
Summary: Tries to apply a command to input.
    First command whose check-function does not return 0 is executed.
*/
void exec_command(char *input)
{
    for (size_t i = 0; i < NUM_COMMANDS; i++)
    {
        int check_code = commands[i].check_handler(input);
        if (check_code != 0)
        {
            if (!commands[i].exec_handler(input, check_code))
            {
                error = true;
            }
            return;
        }
    }
}
