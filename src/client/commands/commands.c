#include <stdio.h>
#include <string.h>

#include "../../util/string_util.h"
#include "../../util/console_util.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../simplification/simplification.h"
#include "../simplification/propositional_context.h"

#include "commands.h"
#include "cmd_evaluation.h"
#include "cmd_help.h"
#include "cmd_clear.h"
#include "cmd_load.h"
#include "cmd_definition.h"
#include "cmd_table.h"

#define COMMENT_PREFIX          '#'
#define QUIT_COMMAND            "quit"

#define SIMPLIFICATION_FILENAME "/simplification.ruleset"

struct Command
{
    /*
    Summary: Checks if passed input string is applicable to command.
    Returns: 0 if command is not applicable, command-specific code otherwise.
    */
    int (*check_handler)(const char *input);
    /*
    Summary: Exec handler is called when check handler returned a value other than 0.
    Params:
        input:      Input string to execute
        check_code: Return value of check handler
    Returns: True if command succeeded, False otherwise (to affect exit code)
    */
    bool (*exec_handler)(char *input, int check_code);
};

static const size_t NUM_COMMANDS = 6;
static const struct Command commands[] = {
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
    unload_simplification();
    unload_console_util();
    unload_history();
    unload_arith_ctx();
    unload_propositional_ctx();
}

/*
Summary: Sets parsing context and initializes commands
*/
void init_commands()
{
    init_arith_ctx();
    init_propositional_ctx();

    #ifdef INSTALL_PATH
        if (init_simplification(INSTALL_PATH SIMPLIFICATION_FILENAME) < 0)
        {
            report_error("Error loading simplification ruleset: " INSTALL_PATH SIMPLIFICATION_FILENAME " not found or readable.\n"
                        "Use 'load simplification <path>' to load a ruleset.\n");
        }
    #else
        whisper("No simplification file loaded, use 'load simplification <path>' to load a ruleset.\n");
    #endif
    
    init_console_util();
    init_history();
}

/*
Summary: Loop to ask user or file for command, ignores comments
    You may want to call set_interactive before.
Returns: True when no command exited with an error, false otherwise
*/
bool process_input(FILE *file)
{
    bool res = true;
    char *input = NULL;
    while (ask_input(file, &input, NULL))
    {
        if (strcmp(input, QUIT_COMMAND) == 0)
        {
            free(input);
            exit(res ? EXIT_SUCCESS : EXIT_FAILURE);
        }
        if (input[0] != '\0' && input[0] != COMMENT_PREFIX)
        {
            if (!exec_command(input))
            {
                res = false;
            }
        }
        free(input);
    }
    // Loop was exited because input was EOF
    whisper("\n");
    return res;
}

/*
Summary: Tries to apply a command to input.
    First command whose check-function does not return 0 is executed.
*/
bool exec_command(char *input)
{
    for (size_t i = 0; i < NUM_COMMANDS; i++)
    {
        int check_code = commands[i].check_handler(input);
        if (check_code != 0)
        {
            return commands[i].exec_handler(input, check_code);
        }
    }
    return false; // To make compiler happy
}
