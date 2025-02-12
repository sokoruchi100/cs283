#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

// takes a string sets the pointer to the first non whitespace character
// then it finds the length of the true string without the trailing space
// copies the trimmed string into a new string and returns it
char *trim_lead_and_trail_spaces(char *string)
{
    for (int i = 0; i < (int)strlen(string); i++)
    {
        if (string[i] != SPACE_CHAR)
        {
            // set the pointer to the first not whitespace char
            string += i;
            break;
        }
    }

    int trueLength = strlen(string);
    for (int i = strlen(string) - 1; i >= 0; i--)
    {
        if (string[i] != SPACE_CHAR)
        {
            trueLength = i + 1;
            break;
        }
    }

    char *trimmedCmd = malloc(trueLength + 1);
    strncpy(trimmedCmd, string, trueLength);
    trimmedCmd[trueLength] = '\0';

    return trimmedCmd;
}

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // split a string by | and iterate through commands
    char *cmd = strtok(cmd_line, PIPE_STRING);
    while (cmd != NULL)
    {
        // handle too many commands
        if (clist->num == CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        // trim the whitespace from the command, remember to free trimmedCmd
        char *trimmedCmd = trim_lead_and_trail_spaces(cmd);

        // store the executable and arguments in the command struct
        command_t newCommand = {0};
        char *res = strchr(trimmedCmd, SPACE_CHAR);

        int exeLength;

        // If no space, there are no arguments in this command, set the length of exe
        if (res == NULL)
        {
            exeLength = strlen(trimmedCmd);
        }
        else
        {
            exeLength = res - trimmedCmd;
        }

        // check that exe length is valid
        if (exeLength > EXE_MAX)
        {
            free(trimmedCmd);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // set the exe command
        strncpy(newCommand.exe, trimmedCmd, exeLength);

        // add arguments if they exist
        if (res != NULL)
        {
            // trim whitespace from arguments
            char *trimmedArgs = trim_lead_and_trail_spaces(res);

            // validate args length
            int argsLength = strlen(trimmedArgs);
            if (argsLength > ARG_MAX)
            {
                free(trimmedArgs);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            // add args to command
            strncpy(newCommand.args, trimmedArgs, argsLength);
            free(trimmedArgs);
        }

        // add command to the command list and increment the count
        clist->commands[clist->num] = newCommand;
        clist->num++;

        cmd = strtok(NULL, PIPE_STRING);

        free(trimmedCmd);
    }

    if (clist->num == 0)
    {
        return WARN_NO_CMDS;
    }

    return OK;
}