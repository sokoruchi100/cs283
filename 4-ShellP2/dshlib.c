#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// helper function to allocate memory
int alloc_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (cmd_buff == NULL)
    {
        return ERR_MEMORY;
    }

    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        cmd_buff->argv[i] = calloc(ARG_MAX, sizeof(char));
        if (cmd_buff->argv[i] == NULL)
        {
            return ERR_MEMORY;
        }
    }

    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    return OK;
}

// helper function to clear the memory
int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        free(cmd_buff->argv[i]);
    }
    free(cmd_buff->_cmd_buffer);
    free(cmd_buff);

    return OK;
}

// clears all parts of the cmd_buff
int clear_cmd_buff(cmd_buff_t *cmd_buff)
{
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        strcpy(cmd_buff->argv[i], "\0");
    }
    strcpy(cmd_buff->_cmd_buffer, "\0");

    return OK;
}

// sets the argc and argv of the command with the proper values
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd)
{
    // copy the original cmd_line to the cmd buffer
    strcpy(cmd->_cmd_buffer, cmd_line);

    // use strtok to get non whitespace tokens of the command
    int totalArgSize = 0;
    char *tok = strtok(cmd_line, SPACE_STRING);
    while (tok != NULL)
    {
        // the first token is the command - Check if it is within the max size
        if (cmd->argc == 0)
        {
            if (strlen(tok) > EXE_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
        }

        // the other tokens are arguments - increment the arg size
        else
        {
            totalArgSize += strlen(tok);
        }

        // store each token string within argv in their correct order
        strcpy(cmd->argv[cmd->argc], tok);

        // as it iterates, increment argc
        cmd->argc++;
        tok = strtok(NULL, SPACE_STRING);
    }

    // check if the arg size went over the max
    if (totalArgSize > ARG_MAX)
    {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    // if there were no iterations, then there was no command
    if (cmd->argc == 0)
    {
        return WARN_NO_CMDS;
    }

    // finally, set the last array position in argv to '\0'
    strcpy(cmd->argv[cmd->argc], "\0");

    return OK;
}

// takes an input string and returns the enum for the built in command
// if not a built in command, returns not a built in command
Built_In_Cmds match_command(const char *input)
{
    if (strcmp(input, EXIT_CMD) == 0)
    {
        return BI_CMD_EXIT;
    }

    if (strcmp(input, DRAGON_CMD) == 0)
    {
        return BI_CMD_DRAGON;
    }

    if (strcmp(input, CD_CMD) == 0)
    {
        return BI_CMD_CD;
    }

    return BI_NOT_BI;
}

// executes the built in command
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds commandCode = match_command(cmd->argv[0]);

    if (commandCode == BI_CMD_EXIT)
    {
        return BI_RC;
    }

    if (commandCode == BI_CMD_DRAGON)
    {
        print_dragon();
    }

    // the cd command should chdir to the provided directory; if no directory is provided, do nothing
    if (commandCode == BI_CMD_CD)
    {
        if (cmd->argc == 2)
        {
            chdir(cmd->argv[1]);
        }
    }

    return BI_EXECUTED;
}

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 *
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    // allocate memory for the cmd_buff
    char *cmd_buff = malloc(SH_CMD_MAX);
    if (cmd_buff == NULL)
    {
        return ERR_MEMORY;
    }

    int rc = 0;

    // initialize the cmd struct
    cmd_buff_t *cmd = malloc(sizeof(cmd_buff_t));
    rc = alloc_cmd_buff(cmd);
    if (rc < 0)
    {
        return ERR_MEMORY;
    }

    while (1)
    {
        // print the shell prompt and get the stdin
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // build the command buffer
        rc = build_cmd_buff(cmd_buff, cmd);

        // Check for any return errors after bulding cmd_list
        if (rc == WARN_NO_CMDS)
        {
            printf(CMD_WARN_NO_CMD);
        }

        if (rc == ERR_CMD_OR_ARGS_TOO_BIG)
        {
            printf("error: command or arguments were too big\n");
        }

        // handle appropriate commands and printing
        if (rc == OK)
        {

            Built_In_Cmds matchedCommand = match_command(cmd->argv[0]);

            // perform built in logic
            if (matchedCommand != BI_NOT_BI)
            {
                Built_In_Cmds cmd_rc = exec_built_in_cmd(cmd);

                // exit command was called
                if (cmd_rc == BI_RC)
                {
                    break;
                }
            }
            // TODO IMPLEMENT if not built-in command, fork/exec as an external command
            // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"
            // Not a built in command, perform fork/exec
            else
            {
            }
        }

        // clear the data within the cmd_buff for the next command
        clear_cmd_buff(cmd);
    }

    // remember to free the memory
    free_cmd_buff(cmd);
    free(cmd_buff);
    return OK;
}