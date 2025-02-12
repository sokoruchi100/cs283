#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"

// Has an encoded dragon array of the {ascii character code, number of times it repeats}
// uncompresses the 2d array and prints it to stdout
void printDragon()
{
    // obtained this from an online decoder and running python scripts until a desired output was achieved
    int compressedDragon[][2] = {{32, 72}, {64, 1}, {37, 4}, {32, 23}, {10, 1}, {32, 69}, {37, 6}, {32, 25}, {10, 1}, {32, 68}, {37, 6}, {32, 26}, {10, 1}, {32, 65}, {37, 1}, {32, 1}, {37, 7}, {32, 11}, {64, 1}, {32, 14}, {10, 1}, {32, 64}, {37, 10}, {32, 8}, {37, 7}, {32, 11}, {10, 1}, {32, 39}, {37, 7}, {32, 2}, {37, 4}, {64, 1}, {32, 9}, {37, 12}, {64, 1}, {32, 4}, {37, 6}, {32, 2}, {64, 1}, {37, 4}, {32, 8}, {10, 1}, {32, 34}, {37, 22}, {32, 6}, {37, 28}, {32, 10}, {10, 1}, {32, 32}, {37, 26}, {32, 3}, {37, 12}, {32, 1}, {37, 15}, {32, 11}, {10, 1}, {32, 31}, {37, 29}, {32, 1}, {37, 19}, {32, 5}, {37, 3}, {32, 12}, {10, 1}, {32, 29}, {37, 28}, {64, 1}, {32, 1}, {64, 1}, {37, 18}, {32, 8}, {37, 2}, {32, 12}, {10, 1}, {32, 28}, {37, 33}, {32, 1}, {37, 22}, {32, 16}, {10, 1}, {32, 28}, {37, 58}, {32, 14}, {10, 1}, {32, 28}, {37, 50}, {64, 1}, {37, 6}, {64, 1}, {32, 14}, {10, 1}, {32, 6}, {37, 8}, {64, 1}, {32, 11}, {37, 16}, {32, 8}, {37, 26}, {32, 6}, {37, 2}, {32, 16}, {10, 1}, {32, 4}, {37, 13}, {32, 9}, {37, 2}, {64, 1}, {37, 12}, {32, 11}, {37, 11}, {32, 1}, {37, 12}, {32, 6}, {64, 1}, {37, 1}, {32, 16}, {10, 1}, {32, 2}, {37, 10}, {32, 3}, {37, 3}, {32, 8}, {37, 14}, {32, 12}, {37, 24}, {32, 24}, {10, 1}, {32, 1}, {37, 9}, {32, 7}, {37, 1}, {32, 9}, {37, 13}, {32, 13}, {37, 12}, {64, 1}, {37, 11}, {32, 23}, {10, 1}, {37, 9}, {64, 1}, {32, 16}, {37, 1}, {32, 1}, {37, 13}, {32, 12}, {64, 1}, {37, 25}, {32, 21}, {10, 1}, {37, 8}, {64, 1}, {32, 17}, {37, 2}, {64, 1}, {37, 12}, {32, 12}, {64, 1}, {37, 28}, {32, 18}, {10, 1}, {37, 7}, {64, 1}, {32, 19}, {37, 15}, {32, 11}, {37, 33}, {32, 14}, {10, 1}, {37, 10}, {32, 18}, {37, 15}, {32, 10}, {37, 35}, {32, 6}, {37, 4}, {32, 2}, {10, 1}, {37, 9}, {64, 1}, {32, 19}, {64, 1}, {37, 14}, {32, 9}, {37, 12}, {64, 1}, {32, 1}, {37, 4}, {32, 1}, {37, 17}, {32, 3}, {37, 8}, {10, 1}, {37, 10}, {32, 18}, {37, 17}, {32, 8}, {37, 13}, {32, 6}, {37, 18}, {32, 1}, {37, 9}, {10, 1}, {37, 9}, {64, 1}, {37, 2}, {64, 1}, {32, 16}, {37, 16}, {64, 1}, {32, 7}, {37, 14}, {32, 5}, {37, 24}, {32, 2}, {37, 2}, {10, 1}, {32, 1}, {37, 10}, {32, 18}, {37, 1}, {32, 1}, {37, 14}, {64, 1}, {32, 8}, {37, 14}, {32, 3}, {37, 26}, {32, 1}, {37, 2}, {10, 1}, {32, 2}, {37, 12}, {32, 2}, {64, 1}, {32, 11}, {37, 18}, {32, 8}, {37, 40}, {32, 2}, {37, 3}, {32, 1}, {10, 1}, {32, 3}, {37, 13}, {32, 1}, {37, 2}, {32, 2}, {37, 1}, {32, 2}, {37, 1}, {64, 1}, {32, 1}, {37, 18}, {32, 10}, {37, 37}, {32, 4}, {37, 3}, {32, 1}, {10, 1}, {32, 4}, {37, 18}, {32, 1}, {37, 22}, {32, 11}, {64, 1}, {37, 31}, {32, 4}, {37, 7}, {32, 1}, {10, 1}, {32, 5}, {37, 39}, {32, 14}, {37, 28}, {32, 8}, {37, 3}, {32, 3}, {10, 1}, {32, 6}, {64, 1}, {37, 35}, {32, 18}, {37, 25}, {32, 15}, {10, 1}, {32, 8}, {37, 32}, {32, 22}, {37, 19}, {32, 2}, {37, 7}, {32, 10}, {10, 1}, {32, 11}, {37, 26}, {32, 27}, {37, 15}, {32, 2}, {64, 1}, {37, 9}, {32, 9}, {10, 1}, {32, 14}, {37, 20}, {32, 11}, {64, 1}, {37, 1}, {64, 1}, {37, 1}, {32, 18}, {64, 1}, {37, 18}, {32, 3}, {37, 3}, {32, 8}, {10, 1}, {32, 18}, {37, 15}, {32, 8}, {37, 10}, {32, 20}, {37, 15}, {32, 4}, {37, 1}, {32, 9}, {10, 1}, {32, 16}, {37, 36}, {32, 22}, {37, 14}, {32, 12}, {10, 1}, {32, 16}, {37, 26}, {32, 2}, {37, 4}, {32, 1}, {37, 3}, {32, 22}, {37, 10}, {32, 2}, {37, 3}, {64, 1}, {32, 10}, {10, 1}, {32, 21}, {37, 19}, {32, 1}, {37, 6}, {32, 1}, {37, 2}, {32, 26}, {37, 13}, {64, 1}, {32, 10}, {10, 1}, {32, 81}, {37, 7}, {64, 1}, {32, 7}, {10, 1}};

    // Calculate how many elements in dragonArray, which is the number of pairs, so divide it by the size of a pair
    int pairs = sizeof(compressedDragon) / sizeof(compressedDragon[0]);

    // iterate through the array and print the character multiple times
    for (int i = 0; i < pairs; i++)
    {
        int value = compressedDragon[i][0];
        int count = compressedDragon[i][1];

        // Print 'value' exactly 'count' times
        for (int j = 0; j < count; j++)
        {
            putchar(value);
        }
    }
}

/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
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
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */
int main()
{
    char *cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    command_list_t clist;
    clist.num = 0;

    while (1)
    {
        // print the shell prompt and get the stdin
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL)
        {
            free(cmd_buff);
            printf("\n");
            break;
        }

        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // build the command list
        rc = build_cmd_list(cmd_buff, &clist);

        // Check for any return errors after bulding cmd_list
        if (rc == WARN_NO_CMDS)
        {
            printf(CMD_WARN_NO_CMD);
        }

        if (rc == ERR_TOO_MANY_COMMANDS)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        }

        if (rc == ERR_CMD_OR_ARGS_TOO_BIG)
        {
            printf("error: command or arguments were too big\n");
        }

        // handle appropriate commands and printing
        if (rc == OK)
        {
            // handle exiting the shell
            if (memcmp(cmd_buff, EXIT_CMD, sizeof(EXIT_CMD)) == 0)
            {
                free(cmd_buff);
                break;
            }

            // handle printing the dragon
            if (memcmp(cmd_buff, "dragon", sizeof("dragon")) == 0)
            {
                printDragon();
            }

            // output the command
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++)
            {
                // no args
                if (strlen(clist.commands[i].args) == 0)
                {
                    printf("<%d> %s\n", i + 1, clist.commands[i].exe);
                }
                else
                {
                    // with args
                    printf("<%d> %s [%s]\n", i + 1, clist.commands[i].exe, clist.commands[i].args);
                }
            }
        }

        // clean up the clist
        clist.num = 0;
        for (int i = 0; i < CMD_MAX; i++)
        {
            command_t emptyCommand = {0};
            clist.commands[i] = emptyCommand;
        }
    }

    // in case of exiting due to EOF
    exit(EXIT_SUCCESS);
}