#include <stdlib.h>
#include <unistd.h>

#include "util/console_util.h"
#include "commands/commands.h"

#include "util/list.h"

/*
 * Scientific calculator in which you can define new functions and constants
 * https://github.com/PhilippHochmann/ccalc
 * (c) 2020 Philipp Hochmann, phil.hochmann[at]gmail[dot]com
 */
int main(int argc, char **argv)
{
    /*LinkedList list = list_create(sizeof(int));
    LIST_PUSH_ELEM(&list, int, 10);
    LIST_PUSH_ELEM(&list, int, -5);
    LIST_PUSH_ELEM(&list, int, -100);
    LIST_PUSH_ELEM(&list, int, 42);
    list_delete(&list, 0);
    list_delete(&list, 2);
    for (size_t i = 0; i < list_count(&list); i++)
    {
        printf("[%zu]: %d, ", i, LIST_GET_ELEM(&list, int, i));
    }
    list_delete(&list, 1);
    list_delete(&list, 0);
    printf("\n");
    for (size_t i = 0; i < list_count(&list); i++)
    {
        printf("[%zu]: %d, ", i, LIST_GET_ELEM(&list, int, i));
    }
    printf("\n");
    list_destroy(&list);*/  

    // Build arithmetic context, initialize commands
    init_commands();
    // Free all resources at exit
    atexit(unload_commands);
    // Parse any arguments non-interactively
    for (int i = 1; i < argc; i++) exec_command(argv[i]);
    // If we are connected to a terminal, use readline and show whispered messages (interactive mode)
    if (isatty(STDIN_FILENO)) set_interactive(true);
    // Enter loop to read all input lines, return appropiate exit code
    return process_input(stdin) ? EXIT_SUCCESS : EXIT_FAILURE;
}
