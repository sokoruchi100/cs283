
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>

// INCLUDES for extra credit
// #include <signal.h>
// #include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

// data to pass to threads
typedef struct
{
    int cli_socket;
    int svr_socket;
} thread_data_t;

atomic_int stop_server_flag = 0; // allows thread to stop the server

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 *
 *      port:   The port the server will use.  Note the constant
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port
 *
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients
 *
 *      This function basically runs the server by:
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server.
 *
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.
 *
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.
 */
int start_server(char *ifaces, int port, int is_threaded)
{
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0)
    {
        int err_code = svr_socket; // server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket, is_threaded);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function.
 *
 *      This function simply returns the value of close() when closing
 *      the socket.
 */
int stop_server(int svr_socket)
{
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.
 *
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically:
 *
 *      1. Create the server socket using the socket() function.
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 *
 *      after creating the socket and prior to calling bind you might want to
 *      include the following code:
 *
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 *
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 *
 *  Returns:
 *
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor,
 *                      which is just an integer.
 *
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails.
 *
 */
int boot_server(char *ifaces, int port)
{
    int svr_socket;
    int ret;

    struct sockaddr_in addr;

    // create a TCP socket for IPv4
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket == -1)
    {
        printf(CMD_ERR_RDSH_COMM);
        return ERR_RDSH_COMMUNICATION;
    }

    // force linux to allow us to reuse the port
    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    // bind the socket to the interface and port
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ifaces);
    addr.sin_port = htons(port);

    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        printf(CMD_ERR_RDSH_COMM);
        return ERR_RDSH_COMMUNICATION;
    }

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(svr_socket, 20);
    if (ret == -1)
    {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *
 *  This function handles managing client connections.  It does this using
 *  the following logic
 *
 *      1.  Starts a while(1) loop:
 *
 *          a. Calls accept() to wait for a client connection. Recall that
 *             the accept() function returns another socket specifically
 *             bound to a client connection.
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.
 *
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop.
 *
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket.
 *
 *  Returns:
 *
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT.
 *
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept()
 *                function failing.
 *
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server.
 *
 */
int process_cli_requests(int svr_socket, int is_threaded)
{
    int cli_socket;
    int rc = OK;

    while (1)
    {
        // stops the server
        if (atomic_load(&stop_server_flag))
        {
            rc = OK_EXIT;
            break;
        }

        // uses the accept syscall to create cli_socket
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket == -1)
        {
            printf(CMD_ERR_RDSH_COMM);
            return ERR_RDSH_COMMUNICATION;
        }

        // if multithreading is enabled, create a new thread
        if (is_threaded)
        {
            // allocate thread data for this connection
            thread_data_t *thread_data = malloc(sizeof(thread_data_t));
            if (!thread_data)
            {
                perror("malloc");
                close(cli_socket);
                continue;
            }
            thread_data->cli_socket = cli_socket;
            thread_data->svr_socket = svr_socket;

            pthread_t thread;
            rc = pthread_create(&thread, NULL, exec_client_requests_threaded, thread_data);
            if (rc != 0)
            {
                printf("Error creating thread %d\n", rc);
                free(thread_data);
                close(cli_socket);
                continue;
            }

            // detach the thread so that its resources are reclaimed upon termination
            pthread_detach(thread);
        }
        else
        {
            // and then execute the client requests
            rc = exec_client_requests(cli_socket);
            if (rc == OK_EXIT)
            {
                printf(RCMD_SERVER_EXITED);
                break;
            }
            else if (rc < 0)
            {
                break;
            }
        }
    }

    return rc;
}

void *exec_client_requests_threaded(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    int rc = exec_client_requests(data->cli_socket);
    if (rc == OK_EXIT)
    {
        // tell it to stop the server immediately
        printf(RCMD_SERVER_EXITED);
        atomic_store(&stop_server_flag, 1);
        shutdown(data->svr_socket, SHUT_RDWR);
    }

    free(data);
    return NULL;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 *
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection.
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop.
 *
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard.
 *
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish
 *  this.
 *
 *  Of final note, this function must allocate a buffer for storage to
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 *
 *  Returns:
 *
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client.
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 *
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors.
 */
int exec_client_requests(int cli_socket)
{
    int io_size;
    command_list_t *cmd_list;
    int rc;
    int cmd_rc;
    int last_rc = 0;
    char *io_buff;
    char *cmd_buff;

    // allocate the command list
    rc = alloc_cmd_list(&cmd_list);
    if (rc != OK)
    {
        perror("malloc");
        return rc;
    }

    // allocate buffers for network recv and command storage
    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    cmd_buff = malloc(SH_CMD_MAX);
    if (io_buff == NULL || cmd_buff == NULL)
    {
        perror("malloc");
        return ERR_RDSH_SERVER;
    }

    while (1)
    {
        // clear the buffer beforehand
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        memset(cmd_buff, 0, SH_CMD_MAX);
        cmd_rc = 0;
        rc = 0;

        // wait for the next data packet
        while ((io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0)) > 0)
        {
            // break if we have an error
            if (io_size < 0)
            {
                rc = ERR_RDSH_COMMUNICATION;
                break;
            }

            // break if no data, other side is closed
            if (io_size == 0)
            {
                break;
            }

            // we have data, append the io_buff to the cmd_buff
            strcat(cmd_buff, io_buff);

            // check if we have the null character, if so we have the end of the command
            if (io_buff[io_size - 1] == '\0')
            {
                // just to avoid the second break;
                rc = 1;
                break;
            }
        }

        // break out of loop if there was an error or no data
        if (rc <= 0)
        {
            break;
        }

        // ensure we clean the command list before processing
        clear_cmd_list(cmd_list);

        // build up the cmd_list struct
        rc = build_cmd_list(cmd_buff, cmd_list);
        if (rc == OK)
        {
            // execute the cmd_list as a pipeline
            cmd_rc = rsh_execute_pipeline(cli_socket, cmd_list);

            if (cmd_rc == EXIT_SC)
            {
                rc = OK;
                break;
            }
            else if (cmd_rc == STOP_SERVER_SC)
            {
                rc = OK_EXIT;
                break;
            }
            else if (cmd_rc == RC_SC)
            {
                char *last_rc_str = calloc(10, 1);
                sprintf(last_rc_str, "%d\n", last_rc);
                send_message_string(cli_socket, last_rc_str);
                free(last_rc_str);
            }
            else
            {
                // send back appropriate response
                rc = send_message_eof(cli_socket);
                if (rc != OK)
                {
                    printf(CMD_ERR_RDSH_COMM);
                    rc = ERR_RDSH_COMMUNICATION;
                    break;
                }
            }

            // save the last command's rc code
            last_rc = cmd_rc;
        }
        else
        {
            switch (rc)
            {
            case WARN_NO_CMDS:
                send_message_string(cli_socket, CMD_WARN_NO_CMD);
                break;
            case ERR_CMD_OR_ARGS_TOO_BIG:
                send_message_string(cli_socket, CMD_ERR_CMD_OR_ARGS_TOO_BIG);
                break;
            case ERR_TOO_MANY_COMMANDS:
                send_message_string(cli_socket, "error: too many commands in pipeline\n");
                break;
            case ERR_MEMORY:
                send_message_string(cli_socket, CMD_ERR_MEMORY);
                break;
            default:
                send_message_string(cli_socket, CMD_ERR_PIPE_FORMAT);
                break;
            }
        }
    }

    // cleanup
    free(io_buff);
    free(cmd_buff);
    free_cmd_list(cmd_list);
    close(cli_socket);

    return rc;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent.
 *
 *  Returns:
 *
 *      OK:  The EOF character was sent successfully.
 *
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character.
 */
int send_message_eof(int cli_socket)
{
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len)
    {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client.
 *
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated.
 *
 *  Returns:
 *
 *      OK:  The message in buff followed by the EOF character was
 *           sent successfully.
 *
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character.
 */
int send_message_string(int cli_socket, char *buff)
{
    // writes the buffer back to the client
    int ret = send(cli_socket, buff, strlen(buff), 0);
    if (ret == -1)
    {
        return ERR_RDSH_COMMUNICATION;
    }

    ret = send_message_eof(cli_socket);
    return ret;
}

// This function sets up redirection for a command in a pipeline or with a socket
void setup_pipeline_redirections(int i, command_list_t *clist, int cli_sock, int pipes[][2])
{
    // if the first command is built in and doesn't have an input file
    if (i == 0 && !clist->commands[i].input_file)
    {
        // link the stdin to the client socket
        dup2(cli_sock, STDIN_FILENO);
    }
    else if (i > 0)
    {
        // link the stdin to the previous pipe
        dup2(pipes[i - 1][0], STDIN_FILENO);
    }

    // if the last command is built in and doesn't have an ouput file
    if (i == clist->num - 1 && !clist->commands[i].output_file)
    {
        // link the stdout and stderr to the client socket
        dup2(cli_sock, STDOUT_FILENO);
        dup2(cli_sock, STDERR_FILENO);
    }
    else if (i < clist->num - 1)
    {
        // link the stdout to the next pipe
        dup2(pipes[i][1], STDOUT_FILENO);
    }
}

// executes the forked command, handles redirection and error checking
int rsh_exec_cmd(command_list_t *clist, int pipes[][2], pid_t *pids, int i, int cli_sock)
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

        setup_pipeline_redirections(i, clist, cli_sock, pipes);

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

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell.
 *
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable.
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:
 *
 *
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │
 *      │   │              │     │              │     │              │  │  │
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │
 *          │              │ │   │              │ │   │              │     │
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘
 *          └──────────────┘     └──────────────┘     └──────────────┘
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function
 *
 *  Returns:
 *
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value.
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist)
{
    int pipes[clist->num - 1][2]; // Array of pipes
    pid_t pids[clist->num];       // Array to store process IDs
    int pids_st[clist->num];      // Array to store process IDs status
    Built_In_Cmds bi_cmd;         // Built in command holder
    int exit_code;                // Exit code that will be returned

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++)
    {
        bi_cmd = rsh_match_command(clist->commands[i].argv[0]);
        if (bi_cmd != BI_NOT_BI)
        {
            // save the original stdin, stdout, and stderr
            int saved_stdin = dup(STDIN_FILENO);
            int saved_stdout = dup(STDOUT_FILENO);
            int saved_stderr = dup(STDERR_FILENO);

            handle_redirection(i, clist);

            setup_pipeline_redirections(i, clist, cli_sock, pipes);

            // built in command, doesn't need to be forked, labeled as -1
            pids[i] = -1;

            // execute the built in command
            bi_cmd = rsh_built_in_cmd(&clist->commands[i]);
            if (bi_cmd == BI_CMD_EXIT)
            {
                // store the exit command
                pids_st[i] = EXIT_SC;
            }
            else if (bi_cmd == BI_CMD_STOP_SVR)
            {
                // store the stop-server command
                pids_st[i] = STOP_SERVER_SC;
            }
            else if (bi_cmd == BI_CMD_RC)
            {
                // store the rc command
                pids_st[i] = RC_SC;
            }
            else
            {
                // handle other built in commands as 0
                pids_st[i] = 0;
            }

            // restore the original stdin, stdout, and stderr
            dup2(saved_stdin, STDIN_FILENO);
            dup2(saved_stdout, STDOUT_FILENO);
            dup2(saved_stderr, STDERR_FILENO);
            close(saved_stdin);
            close(saved_stdout);
            close(saved_stderr);
        }
        else
        {
            // not a built in command, perform fork/exec
            int rc = rsh_exec_cmd(clist, pipes, pids, i, cli_sock);
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
    for (int i = 0; i < clist->num; i++)
    {
        // wait for forked children only
        if (pids[i] != -1)
        {
            waitpid(pids[i], &pids_st[i], 0);
        }
    }

    // by default get exit code of last process
    // use this as the return value
    if (pids[clist->num - 1] != -1)
    {
        exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    }
    else
    {
        exit_code = pids_st[clist->num - 1];
    }
    // check if any of the commands in the pipeline are EXIT_SC or STOP_SERVER_SC
    for (int i = 0; i < clist->num; i++)
    {
        if (pids_st[i] == EXIT_SC)
        {
            exit_code = EXIT_SC;
        }

        if (pids_st[i] == STOP_SERVER_SC)
        {
            exit_code = STOP_SERVER_SC;
        }
    }
    return exit_code;
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 ****
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 *
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 *
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation.
 *
 *  Returns:
 *
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 *
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned.
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the
 *            parsed version fo the command
 *
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.
 *
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.
 *
 *  Returns:
 *
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 *
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY.
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype)
    {
    case BI_CMD_DRAGON:
        print_dragon();
        return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}