#ifndef __DSHLIB_H__
#define __DSHLIB_H__

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX (CMD_MAX + 1)
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct cmd_buff
{
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

/* WIP - Move to next assignment
#define N_ARG_MAX    15     //MAX number of args for a command
typedef struct command{
    char exe [EXE_MAX];
    char args[ARG_MAX];
    int  argc;
    char *argv[N_ARG_MAX + 1];  //last argv[LAST] must be \0
}command_t;
*/

typedef struct command_list
{
    int num;
    cmd_buff_t commands[CMD_MAX];
} command_list_t;

// Special character #defines
#define SPACE_CHAR ' '
#define PIPE_CHAR '|'
#define PIPE_STRING "|"

#define SH_PROMPT "dsh3> "
#define EXIT_CMD "exit"
#define DRAGON_CMD "dragon"
#define CD_CMD "cd"
#define RC_CMD "rc"
#define EXIT_SC 99

// Standard Return Codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3
#define ERR_CMD_ARGS_BAD -4 // for extra credit
#define ERR_MEMORY -5
#define ERR_EXEC_CMD -6
#define OK_EXIT -7

// prototypes
int alloc_cmd_buff(cmd_buff_t *cmd_buff);
int free_cmd_buff(cmd_buff_t *cmd_buff);
int clear_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff);
int close_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_list(char *cmd_line, command_list_t *clist);
int free_cmd_list(command_list_t *cmd_lst);
int clear_cmd_list(command_list_t *cmd_buff);
int str_trim_cpy(char *newStr, char *oldStr);
char *get_next_token(char **p, int *tokenLen);
int validate_token_length(cmd_buff_t *cmd, int tokenLen, int *totalArgLen);
int add_token(cmd_buff_t *cmd, char *tokenStart, int tokenLen);
int parse_cmd_line(cmd_buff_t *cmd, char *trimmed);

// built in command stuff
typedef enum
{
    BI_CMD_EXIT,
    BI_CMD_DRAGON,
    BI_CMD_CD,
    BI_NOT_BI,
    BI_EXECUTED,
    BI_RC,
} Built_In_Cmds;
Built_In_Cmds match_command(const char *input);
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd);
extern void print_dragon();

// main execution context
int exec_local_cmd_loop();
int exec_cmd(command_list_t *clist, int pipes[][2], pid_t *pids, int i);
int execute_pipeline(command_list_t *clist);
int process_cmd_list(char *cmd_buff, command_list_t *cmd_list);
int get_input(char *cmd_buff);
void cleanup_shell(command_list_t *cmd_list, char *cmd_buff);
int init_shell(command_list_t **pCmdList, char **pCmdBuff);

// output constants
#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"
#define CMD_ERR_PIPE_FORMAT "error: piping is improperly formatted\n"
#define CMD_ERR_CMD_OR_ARGS_TOO_BIG "error: command or arguments were too big\n"
#define CMD_ERR_FORK "error: could not fork the process\n"
#define CMD_ERR_EXECUTE "error: could not execute the program\n"
#define CMD_ERR_MEMORY "error: could not allocate memory\n"

// errno related output constants
#define CMD_ERR_EPERM "Operation not permitted\n"
#define CMD_ERR_ENOENT "Command not found in PATH\n"
#define CMD_ERR_EACCES "Permission denied\n"
#define CMD_ERR_E2BIG "Argument list too long\n"
#define CMD_ERR_ENOEXEC "Exec format error\n"
#define CMD_ERR_EISDIR "Is a directory\n"

#endif