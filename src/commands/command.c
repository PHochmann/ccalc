#include "command.h"

Command get_command(CommandInitHandler initHandler, CommandCheckHandler checkHandler, CommandExecHandler execHandler, char *description)
{
    return (Command){
        .initHandler = initHandler,
        .checkHandler = checkHandler,
        .execHandler = execHandler,
        .description = description
    };
}