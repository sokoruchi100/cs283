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
int str_trim_cpy(char *newStr, char *oldStr)
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
    // nullify every string in the array for now, this will be built in the build function
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        if (cmd_buff->argv[i] != NULL)
        {
            free(cmd_buff->argv[i]);
            cmd_buff->argv[i] = NULL;
        }
    }

    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    return OK;
}

// helper function to free the memory
int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        if (cmd_buff->argv[i] != NULL)
        {
            free(cmd_buff->argv[i]);
            cmd_buff->argv[i] = NULL;
        }
    }

    free(cmd_buff->_cmd_buffer);
    free(cmd_buff);

    return OK;
}

// clears all parts of the cmd_buff
int clear_cmd_buff(cmd_buff_t *cmd_buff)
{
    cmd_buff->argc = 0;

    // clears argv by freeing char pointers and setting them to null
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        if (cmd_buff->argv[i] != NULL)
        {
            free(cmd_buff->argv[i]);
            cmd_buff->argv[i] = NULL;
        }
    }
    strcpy(cmd_buff->_cmd_buffer, "\0");

    return OK;
}

char *get_next_token(char **p, int *tokenLen)
{
    // skip any leading spaces
    while (**p == SPACE_CHAR)
    {
        (*p)++;
    }
    if (**p == '\0')
    {
        return NULL;
    }
    char *start;
    int len = 0;

    // handle quoted tokens
    if (**p == '\"')
    {
        (*p)++; // skip the opening quote
        start = *p;
        while (**p != '\0' && **p != '\"')
        {
            (*p)++;
            len++;
        }
        if (**p == '\"')
        {
            (*p)++; // skip the closing quote
        }
    }
    // handle unquoted tokens
    else
    {
        start = *p;
        while (**p != '\0' && **p != SPACE_CHAR)
        {
            (*p)++;
            len++;
        }
    }
    if (tokenLen)
    {
        *tokenLen = len;
    }
    return start;
}

int validate_token_length(cmd_buff_t *cmd, int tokenLen, int *totalArgLen)
{
    if (cmd->argc == 0)
    {
        if (tokenLen > EXE_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
    }
    else
    {
        *totalArgLen += tokenLen;
        if (*totalArgLen > ARG_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
    }
    return OK;
}

int add_token(cmd_buff_t *cmd, char *tokenStart, int tokenLen)
{
    cmd->argv[cmd->argc] = malloc(tokenLen + 1);
    if (cmd->argv[cmd->argc] == NULL)
    {
        return ERR_MEMORY;
    }
    strncpy(cmd->argv[cmd->argc], tokenStart, tokenLen);
    cmd->argv[cmd->argc][tokenLen] = '\0';
    cmd->argc++;

    return OK;
}

int parse_cmd_line(cmd_buff_t *cmd, char *trimmed)
{
    // set up pointer for token parsing
    char *p = trimmed;
    int totalArgLen = 0;
    cmd->argc = 0;
    int rc = 0;

    while (*p != '\0')
    {
        int tokenLen = 0;
        char *tokenStart = get_next_token(&p, &tokenLen);
        if (tokenStart == NULL)
        {
            rc = OK;
            break;
        }

        // validate token length for the command or arguments
        int validateRc = validate_token_length(cmd, tokenLen, &totalArgLen);
        if (validateRc < 0)
        {
            rc = validateRc;
            break;
        }

        // allocate memory for this token and copy it
        int addTokenRc = add_token(cmd, tokenStart, tokenLen);
        if (addTokenRc < 0)
        {
            rc = ERR_MEMORY;
            break;
        }
    }

    return rc;
}

// sets the argc and argv of the command with the proper values
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd)
{
    // copy the original cmd_line to the cmd buffer
    strcpy(cmd->_cmd_buffer, cmd_line);

    // trim whitespace and copy it to the new string
    char *trimmed = malloc(strlen(cmd_line) + 1);
    int trimRc = str_trim_cpy(trimmed, cmd_line);
    if (trimRc != OK)
    {
        free(trimmed);
        return trimRc;
    }

    // parse the cmd_line with the trimmed version
    int parseCmdLineRc = parse_cmd_line(cmd, trimmed);

    free(trimmed);

    // check for any errors returned
    if (parseCmdLineRc < 0)
    {
        return parseCmdLineRc;
    }

    // if there were no tokens, then send the warning
    if (cmd->argc == 0)
    {
        return WARN_NO_CMDS;
    }

    // finally, set the last array position to be null
    cmd->argv[cmd->argc] = NULL;

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

    switch (commandCode)
    {
    case BI_CMD_DRAGON:
        print_dragon();
        return BI_CMD_DRAGON;
    case BI_CMD_CD:
        if (cmd->argc == 2)
        {
            chdir(cmd->argv[1]);
        }
        return BI_CMD_CD;
    default:
        return commandCode;
    }
}

// initializes the shell resources (command buffer and command structure)
// on success, *pCmd and *pCmdBuff are allocated. Returns OK on success
int init_shell(cmd_buff_t **pCmd, char **pCmdBuff)
{
    *pCmdBuff = malloc(SH_CMD_MAX);
    if (*pCmdBuff == NULL)
    {
        return ERR_MEMORY;
    }

    *pCmd = malloc(sizeof(cmd_buff_t));
    if (*pCmd == NULL)
    {
        free(*pCmdBuff);
        return ERR_MEMORY;
    }

    int rc = alloc_cmd_buff(*pCmd);
    if (rc != OK)
    {
        free(*pCmd);
        free(*pCmdBuff);
        return ERR_MEMORY;
    }

    return OK;
}

// cleans up shell resources
void cleanup_shell(cmd_buff_t *cmd, char *cmd_buff)
{
    free_cmd_buff(cmd);
    free(cmd_buff);
}

// reads input from stdin, prints the prompt, and removes the trailing newline
// returns 0 if input was read, returns -1 on EOF to exit early
int get_input(char *cmd_buff)
{
    printf("%s", SH_PROMPT);
    if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
    {
        printf("\n");
        return EOF;
    }
    cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
    return OK;
}

// clears and builds the command from the input buffer
// returns OK if a valid command was built, or an error/warning code
int process_cmd(char *cmd_buff, cmd_buff_t *cmd)
{
    clear_cmd_buff(cmd);
    int rc = build_cmd_buff(cmd_buff, cmd);
    if (rc == WARN_NO_CMDS)
    {
        printf(CMD_WARN_NO_CMD);
    }
    else if (rc == ERR_CMD_OR_ARGS_TOO_BIG)
    {
        printf(CMD_ERR_CMD_OR_ARGS_TOO_BIG);
    }
    return rc;
}

// forks and execs the command, only called if the built in command failed
int exec_cmd(cmd_buff_t *cmd)
{
    int f_result = fork();
    if (f_result < 0)
    {
        printf(CMD_ERR_FORK);
        return ERR_MEMORY;
    }
    else if (f_result == 0)
    {
        // Child process execvp replaces the process
        int childRc = execvp(cmd->argv[0], cmd->argv);
        if (childRc < 0)
        {
            printf(CMD_ERR_EXECVP);
            exit(ERR_EXEC_CMD);
        }
    }
    else
    {
        // Parent process waits for child
        int c_result;
        wait(&c_result);
    }

    return OK;
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
    char *cmd_buff = NULL;
    cmd_buff_t *cmd = NULL;
    int rc = init_shell(&cmd, &cmd_buff);
    if (rc != OK)
    {
        return rc;
    }

    while (1)
    {
        // read input
        if (get_input(cmd_buff) == EOF)
        {
            break;
        }

        // process the input into tokens
        rc = process_cmd(cmd_buff, cmd);
        if (rc != OK)
        {
            continue;
        }

        // execute the command
        Built_In_Cmds cmd_rc = exec_built_in_cmd(cmd);
        if (cmd_rc == BI_CMD_EXIT)
        {
            break;
        }
        else if (cmd_rc == BI_NOT_BI)
        {
            exec_cmd(cmd);
        }
    }

    cleanup_shell(cmd, cmd_buff);
    return OK;
}