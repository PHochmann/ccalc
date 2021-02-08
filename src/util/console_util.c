#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "console_util.h"
#include "string_builder.h"

bool interactive;

void unload_console_util()
{
#ifdef USE_READLINE
    rl_clear_history();
#endif
}

void init_console_util()
{
    interactive = false;
#ifdef USE_READLINE
    // Disable tab completion
    rl_bind_key('\t', rl_insert);
#endif
}

/*
Summary: Sets interactive mode
    When true, whispered messages are displayed and readline instead of getline is used to read input
*/
bool set_interactive(bool value)
{
    bool res = interactive;
    interactive = value;
    return res;
}

bool is_interactive()
{
    return interactive;
}

/*
Summary: printf-wrapper that filters unimportant prints in non-interactive mode
*/
void whisper(const char *format, ...)
{
    if (interactive)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

bool ask_input_getline(FILE *file, char **out_input, const char *prompt_fmt, va_list args)
{
    if (interactive && prompt_fmt != NULL)
    {
        vprintf(prompt_fmt, args);
    }
    
    size_t size;
    *out_input = NULL;
    if (getline(out_input, &size, file) == EOF)
    {
        free(*out_input);
        *out_input = NULL;
        return false;
    }

    // Overwrite newline char
    (*out_input)[strlen(*out_input) - 1] = '\0';
    return true;
}


/*
Summary: Used whenever input is requested. Prompt is only printed when interactive.
Params
    prompt: Prompt to display when interactive
    file: Used when not interactive - should be stdin when arguments are piped in or file when load command is used
    out_input: Pointer to string that will be read. String must be free'd after use.
*/
#ifdef USE_READLINE

// File is stdin, interactive is true
bool ask_input_readline(char **out_input, const char *prompt_fmt, va_list args)
{
    if (prompt_fmt != NULL)
    {
        Vector strbuilder = strbuilder_create(3);
        vstrbuilder_append(&strbuilder, prompt_fmt, args);
        *out_input = readline(strbuilder.buffer);
        vec_destroy(&strbuilder);
    }
    else
    {
        *out_input = readline(NULL);
    }

    if (*out_input == NULL)
    {
        return false;
    }
    add_history(*out_input);
    return true;
}

bool vask_input(FILE *file, char **out_input, const char *prompt_fmt, va_list args)
{
    // Use readline when interactive
    if (interactive)
    {
        return ask_input_readline(out_input, prompt_fmt, args);
    }
    else
    {
        return ask_input_getline(file, out_input, prompt_fmt, args);
    }
}

#else

bool vask_input(FILE *file, char **out_input, const char *prompt_fmt, va_list args)
{
    return ask_input_getline(file, out_input, prompt_fmt, args);
}

#endif

bool ask_input(FILE *file, char **out_input, const char *prompt_fmt, ...)
{
    va_list args;
    va_start(args, prompt_fmt);
    bool res;
    res = vask_input(file, out_input, prompt_fmt, args);
    va_end(args);
    return res;
}

void report_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void software_defect(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Software defect: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void show_error_with_position(int pos, int length, const char *fmt, ...)
{
    if (is_interactive())
    {
        report_error("%*s^", pos, "");
        for (int i = 0; i < length - 1; i++)
        {
            report_error("~");
        }
    }
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, " ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}
