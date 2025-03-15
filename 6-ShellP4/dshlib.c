#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

/****
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

// global variable to hold the last error return code for the child process
int last_rc = 0;

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
        }
        cmd_buff->argv[i] = NULL;
    }
    cmd_buff->argc = 0;

    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL)
    {
        return ERR_MEMORY;
    }
    cmd_buff->_cmd_buffer[0] = '\0'; // initialize as empty string
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

int clear_cmd_list(command_list_t *cmd_buff)
{
    for (int i = 0; i < CMD_MAX; i++)
    {
        clear_cmd_buff(&cmd_buff->commands[i]);
    }
    cmd_buff->num = 0;
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

    // allocate a temporary buffer large enough for the token
    int maxLen = strlen(*p);
    char *token = malloc(maxLen + 1);
    if (!token)
    {
        return NULL; // memory allocation error
    }
    int pos = 0;

    // read until a whitespace is encountered
    while (**p != '\0' && **p != SPACE_CHAR)
    {
        // handle quotes within the token
        if (**p == '\"') // handle double quotes
        {
            // skip the quote and keep copying until we reach the closing quote
            (*p)++; // Skip the opening quote
            while (**p != '\0' && **p != '\"')
            {
                token[pos++] = **p;
                (*p)++;
            }
            if (**p == '\"')
            {
                (*p)++; // skip the closing quote
            }
        }
        else if (**p == '\'') // handle single quotes
        {
            // skip the quote and keep copying until we reach the closing quote
            (*p)++; // Skip the opening quote
            while (**p != '\0' && **p != '\'')
            {
                token[pos++] = **p;
                (*p)++;
            }
            if (**p == '\'')
            {
                (*p)++; // skip the closing quote
            }
        }
        else
        {
            // regular character: copy it
            token[pos++] = **p;
            (*p)++;
        }
    }
    // cap the token with a null terminator
    token[pos] = '\0';
    if (tokenLen)
    {
        *tokenLen = pos;
    }
    return token;
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
        char *token = get_next_token(&p, &tokenLen);
        if (token == NULL)
        {
            rc = OK;
            break;
        }

        // validate token length for the command or arguments
        int validateRc = validate_token_length(cmd, tokenLen, &totalArgLen);
        if (validateRc < 0)
        {
            rc = validateRc;
            free(token);
            break;
        }

        int addTokenRc = add_token(cmd, token, tokenLen);
        free(token); // free the temporary token after adding it
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
    if (cmd_line == NULL)
    {
        return WARN_NO_CMDS;
    }

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

    if (strcmp(input, RC_CMD) == 0)
    {
        return BI_CMD_RC;
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
    case BI_CMD_RC:
        // print the last return code
        printf("%d\n", last_rc);
        return BI_CMD_RC;
    default:
        return commandCode;
    }
}

// initializes the shell resources (command buffer and command structure)
// on success, *pCmd and *pCmdBuff are allocated. Returns OK on success
int alloc_cmd_list(command_list_t **pCmdList)
{
    *pCmdList = malloc(sizeof(command_list_t));
    if (*pCmdList == NULL)
    {
        return ERR_MEMORY;
    }
    memset(*pCmdList, 0, sizeof(command_list_t)); // zero out the memory

    // allocate the command buffer for each command in the list
    for (int i = 0; i < CMD_MAX; i++)
    {
        int rc = alloc_cmd_buff(&(*pCmdList)->commands[i]);
        if (rc != OK)
        {
            // free them before returning an error
            for (int j = 0; j < i; j++)
            {
                free_cmd_buff(&(*pCmdList)->commands[j]);
            }
            free(*pCmdList);
            return ERR_MEMORY;
        }
    }

    return OK;
}

int free_cmd_list(command_list_t *cmd_lst)
{
    for (int i = 0; i < CMD_MAX; i++)
    {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    free(cmd_lst);
    return OK;
}

// reads input from stdin, prints the prompt, and removes the trailing newline
// returns 0 if input was read, returns -1 on EOF to exit early
int get_input(char *cmd_buff)
{
    if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
    {
        printf("\n");
        return EOF;
    }
    cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
    return OK;
}

// builds the cmd_list and its buffers handles errors
int build_cmd_list(char *cmd_line, command_list_t *cmd_list)
{
    // run strtok on cmd_list, seperate by pipes
    char *tok = strtok(cmd_line, PIPE_STRING);

    // iterate through each command token
    int rc = 0;
    while (tok != NULL)
    {
        // error check for too many commands piped
        if (cmd_list->num >= CMD_MAX)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            return ERR_TOO_MANY_COMMANDS;
        }

        // process it as a command buffer and store it into the cmd_list, incrementing the count
        int rc = build_cmd_buff(tok, &cmd_list->commands[cmd_list->num]);
        if (rc == WARN_NO_CMDS)
        {
            printf(CMD_ERR_PIPE_FORMAT);
            return ERR_CMD_ARGS_BAD;
        }
        else if (rc == ERR_CMD_OR_ARGS_TOO_BIG)
        {
            printf(CMD_ERR_CMD_OR_ARGS_TOO_BIG);
            return rc;
        }

        cmd_list->num++;
        tok = strtok(NULL, PIPE_STRING);
    }

    if (cmd_list->num == 0)
    {
        printf(CMD_WARN_NO_CMD);
        return WARN_NO_CMDS;
    }

    return rc;
}

// prints the associated error message for each error type
void output_exec_error(int err)
{
    switch (err)
    {
    case EPERM:
        printf(CMD_ERR_EPERM);
        break;
    case ENOENT:
        printf(CMD_ERR_ENOENT);
        break;
    case EACCES:
        printf(CMD_ERR_EACCES);
        break;
    case E2BIG:
        printf(CMD_ERR_E2BIG);
        break;
    case ENOEXEC:
        printf(CMD_ERR_ENOEXEC);
        break;
    case EISDIR:
        printf(CMD_ERR_EISDIR);
        break;
    default:
        printf(CMD_ERR_EXECUTE);
        break;
    }
}

// helper to handle input redirection ("<")
int perform_input_redirection(char **argv_ptr)
{
    // check if the filename is missing
    if (argv_ptr[1] == NULL)
    {
        printf(CMD_ERR_REDIRECTION_FORMAT);
        exit(ERR_CMD_ARGS_BAD);
    }
    int fd = open(argv_ptr[1], O_RDONLY);
    if (fd < 0)
    {
        int err = errno;
        output_exec_error(err);
        exit(err);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
    // set to null to prevent rest of arguments from being read
    argv_ptr[0] = NULL;
    return OK;
}

// Helper to handle output redirection (">" or ">>")
// 'flags' should be set appropriately for truncation or appending.
int perform_output_redirection(char **argv_ptr, int flags)
{
    // check if the filename is missing
    if (argv_ptr[1] == NULL)
    {
        printf(CMD_ERR_REDIRECTION_FORMAT);
        exit(ERR_CMD_ARGS_BAD);
    }
    int fd = open(argv_ptr[1], flags, 0644);
    if (fd < 0)
    {
        int err = errno;
        output_exec_error(err);
        exit(err);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    // set to null to prevent rest of arguments from being read
    argv_ptr[0] = NULL;
    return OK;
}

// handles the redirection of input and output for the command, also append
int handle_redirection(int i, command_list_t *clist)
{
    cmd_buff_t *cmd = &clist->commands[i];

    // If this is the first command, check for input redirection.
    if (i == 0)
    {
        for (int j = 0; j < cmd->argc; j++)
        {
            if (strcmp(cmd->argv[j], "<") == 0)
            {
                clist->commands[i].input_file = cmd->argv[j + 1];
                return perform_input_redirection(&cmd->argv[j]);
            }
        }
    }

    // If this is the last command, check for output redirection.
    if (i == clist->num - 1)
    {
        for (int j = 0; j < cmd->argc; j++)
        {
            if (strcmp(cmd->argv[j], ">") == 0)
            {
                clist->commands[i].output_file = cmd->argv[j + 1];
                return perform_output_redirection(&cmd->argv[j], O_WRONLY | O_CREAT | O_TRUNC);
            }
            else if (strcmp(cmd->argv[j], ">>") == 0)
            {
                clist->commands[i].output_file = cmd->argv[j + 1];
                return perform_output_redirection(&cmd->argv[j], O_WRONLY | O_CREAT | O_APPEND);
            }
        }
    }

    return OK;
}

// forks and execs the command, only called if the built in command failed
int exec_cmd(command_list_t *clist, int pipes[][2], pid_t *pids, int i)
{
    pids[i] = fork();
    if (pids[i] < 0)
    {
        printf(CMD_ERR_FORK);
        return ERR_MEMORY;
    }

    // this is a child process
    if (pids[i] == 0)
    {
        handle_redirection(i, clist);

        // if not the first command, set the input to the previous pipe
        if (i > 0)
        {
            dup2(pipes[i - 1][0], STDIN_FILENO);
        }

        // if not the last command, set the output to the next pipe
        if (i < clist->num - 1)
        {
            dup2(pipes[i][1], STDOUT_FILENO);
        }

        // Close all pipe ends in child
        for (int j = 0; j < clist->num - 1; j++)
        {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }

        // execute the command
        int childRc = execvp(clist->commands[i].argv[0], clist->commands[i].argv);
        // Check for common file related error codes
        if (childRc < 0)
        {
            int err = errno;
            output_exec_error(err);
            exit(err); // exit with the error code
        }
    }

    return OK;
}

// executes the pipeline of commands, built in commands are not forked and piped
int execute_pipeline(command_list_t *clist)
{
    // extra check for just the exit command
    if (clist->num == 1 && match_command(clist->commands[0].argv[0]) == BI_CMD_EXIT)
    {
        return OK_EXIT;
    }

    // create an array of pipes and an array of process ids
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];

    // create the pipes
    for (int i = 0; i < clist->num - 1; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            return ERR_MEMORY;
        }
    }

    // create processes for each command
    for (int i = 0; i < clist->num; i++)
    {
        Built_In_Cmds cmd_rc = exec_built_in_cmd(&clist->commands[i]);
        if (cmd_rc == BI_NOT_BI)
        {
            int rc = exec_cmd(clist, pipes, pids, i);
            if (rc < 0)
            {
                return rc;
            }
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    int status;
    for (int i = 0; i < clist->num; i++)
    {
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status))
        {
            last_rc = WEXITSTATUS(status);
        }
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

    // declare the command buffer and command list
    char *cmd_buff = NULL;
    command_list_t *cmd_list = NULL;

    int rc = alloc_cmd_list(&cmd_list);
    if (rc != OK)
    {
        return rc;
    }
    cmd_buff = malloc(SH_CMD_MAX);
    if (cmd_buff == NULL)
    {
        free_cmd_list(cmd_list);
        return ERR_MEMORY;
    }

    while (1)
    {
        printf("%s", SH_PROMPT);
        // read input
        if (get_input(cmd_buff) == EOF)
        {
            break;
        }

        // ensure we clean the command list before processing
        clear_cmd_list(cmd_list);

        // process the cmd list of multiple commands being piped
        rc = build_cmd_list(cmd_buff, cmd_list);
        if (rc != OK)
        {
            continue;
        }

        // execute the commands in the list as a pipeline
        int rc = execute_pipeline(cmd_list);
        if (rc == OK_EXIT)
        {
            break;
        }
        else if (rc == ERR_MEMORY)
        {
            printf(CMD_ERR_MEMORY);
            return rc;
        }
    }

    free_cmd_list(cmd_list);
    free(cmd_buff);
    return rc;
}