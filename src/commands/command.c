#include "command.h"

Command get_command(CommandInitHandler initHandler, CommandCheckHandler checkHandler, CommandExecHandler execHandler)
{
    return (Command){
        .initHandler = initHandler,
        .checkHandler = checkHandler,
        .execHandler = execHandler
    };
}