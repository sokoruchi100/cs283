#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// takes a string sets the pointer to the first non whitespace character
// then it finds the length of the true string without the trailing space
// copies the trimmed string into a new string and returns it
char *trim_lead_and_trail_spaces(char *string)
{
    // just in case to help in loop logic
    if (string == NULL)
    {
        return NULL;
    }

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

    // trim whitespace from the front and back
    char *trimmedCmd = trim_lead_and_trail_spaces(cmd_line);

    // useful variables to notify the start and end of each word
    char *start = trimmedCmd;
    char *end;
    int lengthOfArgs = 0;
    while (start != NULL)
    {
        // start by obtaining the end char position
        end = strchr(start, SPACE_CHAR);

        // Obtain the length of the word
        int wordLength;
        if (end == NULL)
        {
            wordLength = strlen(start);
        }
        else
        {
            wordLength = end - start;
        }

        // the word is the first command, check its length for validitiy
        if (cmd->argc == 0)
        {
            if (wordLength > EXE_MAX)
            {
                free(trimmedCmd);
                free(start);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
        }
        // this is an argument word instead, increment the length and check if it is too big
        else
        {
            // if the current word is a quote, find the next quote location in the cmd_line, and add the entire string from quote to quote to a running string
            if (*start == '\"')
            {
                start++;

                // if the start + 1 is invalid, the quote has no end quote, for now, assume that is not the case
                end = strchr(start, '\"');
                wordLength = end - start;
            }

            // continuously add the strings to a running size and do size checks of ARG_MAX
            lengthOfArgs += wordLength;
            if (lengthOfArgs > ARG_MAX)
            {
                free(trimmedCmd);
                free(start);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
        }

        // copy the word into argv and end it with a null terminator
        strncpy(cmd->argv[cmd->argc], start, wordLength);
        cmd->argv[cmd->argc][wordLength] = '\0';

        // increment the number of words in the command
        cmd->argc++;

        // continue looping to the next word
        free(start);
        start = trim_lead_and_trail_spaces(end);
    }

    // remember to free strings once no longer needed
    free(start);

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
    int exitFlag = 0;

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
        switch (rc)
        {
        case WARN_NO_CMDS:
            printf(CMD_WARN_NO_CMD);
            break;
        case ERR_CMD_OR_ARGS_TOO_BIG:
            printf(CMD_ERR_CMD_OR_ARGS_TOO_BIG);
            break;
        default:
            Built_In_Cmds matchedCommand = match_command(cmd->argv[0]);

            // perform built in logic
            if (matchedCommand != BI_NOT_BI)
            {
                Built_In_Cmds cmd_rc = exec_built_in_cmd(cmd);

                // exit command was called
                if (cmd_rc == BI_RC)
                {
                    exitFlag = 1;
                }
            }
            // TODO IMPLEMENT if not built-in command, fork/exec as an external command
            // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"
            // Not a built in command, perform fork/exec
            else
            {
                
            }
            break;
        }

        // clear the data within the cmd_buff for the next command
        clear_cmd_buff(cmd);

        if (exitFlag)
        {
            break;
        }
    }

    // remember to free the memory
    free_cmd_buff(cmd);
    free(cmd_buff);
    return OK;
}