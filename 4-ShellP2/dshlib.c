#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// copies the oldStr into the newStr without any of the leading and trailing whitespace
// does not mutate the old string or the new string
// returns a return code in case bugs crop up
int strtrimcpy(char *newStr, char *oldStr)
{
    // check for null pointers
    if (oldStr == NULL || newStr == NULL)
    {
        return ERR_MEMORY;
    }

    int length = strlen(oldStr);
    int start = 0;

    // find the first character non whitespace char
    while (start < length && oldStr[start] == SPACE_CHAR)
    {
        start++;
    }

    // if the string is all spaces, newStr is empty
    if (start == length)
    {
        newStr[0] = '\0';
        return OK;
    }

    int end = length - 1;

    // find the last character non whitespace char
    while (end > start && oldStr[end] == SPACE_CHAR)
    {
        end--;
    }

    int trueLength = end - start + 1;

    // copy substring from start all for the true length of characters
    strncpy(newStr, oldStr + start, trueLength);
    newStr[trueLength] = '\0';

    return OK;
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

    // allocate a buffer to hold the trimmed command
    char *trimmed = malloc(strlen(cmd_line) + 1);
    if (trimmed == NULL)
    {
        return ERR_MEMORY;
    }

    // trim whitespace and copy it to the new string
    int trimRc = strtrimcpy(trimmed, cmd_line);
    if (trimRc != OK)
    {
        free(trimmed);
        return trimRc;
    }

    // set up pointer for token parsing
    char *p = trimmed;
    int lengthOfArgs = 0;
    cmd->argc = 0;
    while (*p != '\0')
    {
        // skip extra space in the token
        while (*p == SPACE_CHAR)
        {
            p++;
        }

        // if we happen to reach the end already, just exit the loop
        if (*p == '\0')
        {
            break;
        }

        // setup the start and lengths for the tokens for copying
        char *tokenStart = p;
        int tokenLength = 0;

        // if the token has a quote, handle the entire quote as a single string
        if (*p == '\"')
        {
            // handle quoted token
            tokenStart = ++p; // skip the opening quote
            while (*p != '\0' && *p != '\"')
            {
                tokenLength++;
                p++;
            }

            // if a closing quote is found, skip it
            if (*p == '\"')
            {
                p++;
            }
        }
        else
        {
            // the token is unquoted, so perform a normal operation
            while (*p != '\0' && *p != SPACE_CHAR)
            {
                tokenLength++;
                p++;
            }
        }

        // validate token length based on whether its the command or an argument
        if (cmd->argc == 0)
        {
            if (tokenLength > EXE_MAX)
            {
                free(trimmed);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
        }
        else
        {
            lengthOfArgs += tokenLength;
            if (lengthOfArgs > ARG_MAX)
            {
                free(trimmed);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
        }

        // copy the token into argv and end it with a null terminator
        strncpy(cmd->argv[cmd->argc], tokenStart, tokenLength);
        cmd->argv[cmd->argc][tokenLength] = '\0';
        cmd->argc++;
    }

    free(trimmed);

    // if there were no tokens, then send the warning
    if (cmd->argc == 0)
    {
        return WARN_NO_CMDS;
    }

    // finally, set the last array position in argv to "\0" string
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

        for (int i = 0; i < cmd->argc; i++)
        {
            printf("%s\n", cmd->argv[i]);
        }

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
                // fork the current shell process
                int f_result, c_result;

                f_result = fork();
                if (f_result < 0)
                {
                    perror("fork error");
                }

                // for the child process, we want to use exec to replace it with the command
                if (f_result == 0)
                {
                    int childRc;
                    childRc = execvp(cmd->argv[0], cmd->argv);
                    if (childRc < 0)
                    {
                        // handle the error in case we failed to execute the command, child process exits prematurely
                        perror("command error");
                        exit(42);
                    }
                }
                // for the parent process, we wait until the child is finished
                else
                {
                    wait(&c_result);
                }
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