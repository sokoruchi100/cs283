#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50

// prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

// prototypes for functions to handle required functionality
int count_words(char *, int, int);
// add additional prototypes here
int reverse_string(char *, int, int);
int print_words(char *, int, int);
int search_and_replace(char *, int, int, int, char *[]);

// Sets up the buffer by copying over every non white space character and a single space in between each word
// Also keeps track of the string length, will return early if larger than the len
// Finally, fills the rest of the buffer with .
int setup_buff(char *buff, char *user_str, int len)
{
    // TODO: #4:  Implement the setup buff as per the directions

    // Finds the length of the inputted string if it were reformatted without extraneous spaces
    int strLen = 0;
    int strIter = 0;
    int firstWordPassed = 0;
    while (*(user_str + strIter) != '\0')
    {
        char currentChar = *(user_str + strIter);
        // Any non white space character increments the string length
        if (currentChar >= 33 && currentChar <= 126)
        {
            // white space copier
            if (firstWordPassed)
            {
                char prevChar = *(user_str + strIter - 1);
                if (prevChar == 32 || prevChar == 9)
                {
                    // safety check when copying
                    if (strLen >= len)
                    {
                        return -1;
                    }
                    *(buff + strLen) = ' ';
                    strLen++;
                }
            }

            // handles leading space edgecase
            if (!firstWordPassed)
            {
                firstWordPassed = 1;
            }

            // safety check when copying
            if (strLen >= len)
            {
                return -1;
            }
            *(buff + strLen) = currentChar;
            strLen++;
        }

        strIter++;
    }

    // fill the rest with .
    for (int i = strLen; i < len; i++)
    {
        *(buff + i) = '.';
    }
    return strLen; // returns the length of the supplied string
}

void print_buff(char *buff, int len)
{
    printf("Buffer:  [");
    for (int i = 0; i < len; i++)
    {
        putchar(*(buff + i));
    }
    printf("]");
    putchar('\n');
}

void usage(char *exename)
{
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

// This function counts the number of words in the string buffer
// It does this by counting the number of spaces, then adding 1 since there are n-1 spaces for n words
// However, if there is only 1 word with no spaces, it just adds 1
int count_words(char *buff, int len, int str_len)
{
    // YOU MUST IMPLEMENT
    int numOfWords = 0;

    // add 1 if there is a non white space character
    for (int i = 0; i < str_len; i++)
    {
        if (*(buff + i) != ' ')
        {
            numOfWords++;
            break;
        }
    }

    // then add 1 for each space
    for (int i = 0; i < str_len; i++)
    {
        if (*(buff + i) == ' ')
        {
            numOfWords++;
        }
    }

    return numOfWords;
}

// ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

// Reverses string in buffer by keeping two pointers on the first and last characters
// It swaps the two characters and progresses each pointer forwards and backwards respectively
// Stops when the first pointer has reached at least half way
// Then copies only the string portion to another block of memory, prints it, then frees it
int reverse_string(char *buff, int len, int str_len)
{
    int head = 0;
    int tail = str_len - 1;
    while (head < str_len / 2)
    {
        char tempHead = *(buff + head);
        *(buff + head) = *(buff + tail);
        *(buff + tail) = tempHead;
        head++;
        tail--;
    }
    char *printedStr = malloc(str_len + 1);
    memcpy(printedStr, buff, str_len);
    *(printedStr + str_len) = '\0';

    printf("Reversed String: %s\n", printedStr);
    free(printedStr);
    return 0;
}

// Prints each word in order along with their length
// Prints the order number upon hitting a new character if prev char was space or first word
// Then prints the entire word as it iterates, counting the length
// Finally prints the length of the word upon hitting a space or the end of the string
// Also prints the number of words by calling the helper function
int print_words(char *buff, int len, int str_len)
{
    printf("Word Print\n");
    printf("----------\n");
    int wordNumber = 1;
    int wordLength = 0;
    for (int i = 0; i < str_len; i++)
    {
        char currentChar = *(buff + i);
        if (i == 0 || *(buff + i - 1) == ' ')
        {
            printf("%d. ", wordNumber);
        }

        if (currentChar != ' ')
        {
            printf("%c", currentChar);
            wordLength++;
        }
        else
        {
            printf("(%d)\n", wordLength);
            wordLength = 0;
            wordNumber++;
        }
    }
    printf("(%d)\n", wordLength);

    int numOfWords = count_words(buff, len, str_len);
    printf("\nNumber of words returned: %d\n", numOfWords);
    return 0;
}

// searches for the word in the 2nd argument and replaces the first occurence of it with the 3rd argument
// iterates through buffer until it finds the start of the search word
// then it checks to see if the word is correct
// finally, it replaces each char of the word with the replace word
// The string shifts accordingly to the differences in word lengths
// if the string shifts pass the buffer length, return an error
int search_and_replace(char *buff, int len, int str_len, int argc, char *argv[])
{
    if (argc != 5)
    {
        return -1;
    }

    // gets words and calculate lengths
    char *searchWord = *(argv + 3);
    int searchWordLength = 0;
    while (*(searchWord + searchWordLength) != '\0')
    {
        searchWordLength++;
    }

    char *replaceWord = *(argv + 4);
    int replaceWordLength = 0;
    while (*(replaceWord + replaceWordLength) != '\0')
    {
        replaceWordLength++;
    }

    int bufIter = 0;
    while (bufIter < str_len)
    {
        char currentChar = *(buff + bufIter);
        if (currentChar == *searchWord)
        {
            // check if the word is correct
            int searchWordIter = 0;
            int differenceFound = 0;
            while (searchWordIter < searchWordLength)
            {
                // prevent illegal memory access, the word wasn't found
                if (bufIter + searchWordIter > len)
                {
                    return -1;
                }

                if (*(buff + bufIter + searchWordIter) != *(searchWord + searchWordIter))
                {
                    differenceFound = 1;
                    break;
                }
                searchWordIter++;
            }

            if (!differenceFound)
            {
                // start replacement
                int replaceWordIter = 0;
                int shiftAmount = replaceWordLength - searchWordLength;
                if (shiftAmount > 0)
                {
                    // shift right, skips if possible overflow
                    int shiftIter = -1;
                    while (str_len + shiftIter >= bufIter + searchWordLength)
                    {
                        if (str_len + shiftIter + shiftAmount >= len)
                        {
                            shiftIter--;
                            continue;
                        }
                        *(buff + str_len + shiftIter + shiftAmount) = *(buff + str_len + shiftIter);
                        shiftIter--;
                    }

                    // replace
                    while (replaceWordIter < replaceWordLength)
                    {
                        *(buff + bufIter + replaceWordIter) = *(replaceWord + replaceWordIter);
                        replaceWordIter++;
                    }
                }
                else if (shiftAmount < 0)
                {
                    // replace
                    while (replaceWordIter < replaceWordLength)
                    {
                        *(buff + bufIter + replaceWordIter) = *(replaceWord + replaceWordIter);
                        replaceWordIter++;
                    }

                    // perform shift left
                    int shiftIter = 0;
                    while (bufIter + replaceWordIter + shiftIter - shiftAmount < str_len)
                    {
                        *(buff + bufIter + replaceWordIter + shiftIter) = *(buff + bufIter + replaceWordIter + shiftIter - shiftAmount);
                        shiftIter++;
                    }

                    // fill the new empty space with .
                    for (int i = bufIter + replaceWordIter + shiftIter; i < len; i++)
                    {
                        *(buff + i) = '.';
                    }
                }
                else
                {
                    // just replace
                    while (replaceWordIter < replaceWordLength)
                    {
                        *(buff + bufIter + replaceWordIter) = *(replaceWord + replaceWordIter);
                        replaceWordIter++;
                    }
                }

                // print the string
                char *printedStr = malloc(str_len + shiftAmount + 1);
                memcpy(printedStr, buff, str_len + shiftAmount);
                *(printedStr + str_len + shiftAmount) = '\0';

                printf("Modified String: %s\n", printedStr);
                free(printedStr);
                return 0;
            }
        }
        bufIter++;
    }

    // word wasn't found
    return -1;
}

int main(int argc, char *argv[])
{

    char *buff;         // placehoder for the internal buffer
    char *input_string; // holds the string provided by the user on cmd line
    char opt;           // used to capture user option from cmd line
    int rc;             // used for return codes
    int user_str_len;   // length of user supplied string

    // TODO:  #1. WHY IS THIS SAFE, aka what if argv[1] does not exist?
    //       PLACE A COMMENT BLOCK HERE EXPLAINING
    /* if argv[1] doesn't exist, as in, there is no argument supplied to the program,
     * then argc will be exactly 1, which has only argv[0] which is the name of the program itself.
     * Since argc < 2, it will call usage and then exit prematurely as a safety mechanism.
     * It can also safely exit if the first argument's starting char is not '-'.
     * In other words, the program needs to have a first argument starting with '-'
     * If the program doesn't get that argument, it will exit safely.
     */
    if ((argc < 2) || (*argv[1] != '-'))
    {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1); // get the option flag

    // handle the help flag and then exit normally
    if (opt == 'h')
    {
        usage(argv[0]);
        exit(0);
    }

    // WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    // TODO:  #2 Document the purpose of the if statement below
    //       PLACE A COMMENT BLOCK HERE EXPLAINING
    /* The if statement below checks if there are less than 2 arguments supplied to the program
     * It says argc < 3 because the name of the program is also counted in argc
     * If so, it calls usage to inform the user about how to use the program correctly
     * Then, it exits with a 1, meaning there was an error
     */
    if (argc < 3)
    {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; // capture the user input string

    // TODO:  #3 Allocate space for the buffer using malloc and
    //           handle error if malloc fails by exiting with a
    //           return code of 99
    //  CODE GOES HERE FOR #3
    buff = malloc(BUFFER_SZ);
    if (buff == NULL)
    {
        printf("Could not allocate memory to buffer!\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ); // see todos
    if (user_str_len < 0)
    {
        printf("Error setting up buffer, error = %d\n", user_str_len);
        exit(2);
    }

    switch (opt)
    {
    case 'c':
        rc = count_words(buff, BUFFER_SZ, user_str_len); // you need to implement
        if (rc < 0)
        {
            printf("Error counting words, rc = %d\n", rc);
            exit(2);
        }
        printf("Word Count: %d\n", rc);
        break;

    // TODO:  #5 Implement the other cases for 'r' and 'w' by extending
    //        the case statement options
    case 'r':
        rc = reverse_string(buff, BUFFER_SZ, user_str_len);
        if (rc < 0)
        {
            printf("Error reversing string, rc = %d\n", rc);
            exit(2);
        }
        break;
    case 'w':
        rc = print_words(buff, BUFFER_SZ, user_str_len);
        if (rc < 0)
        {
            printf("Error printing words, rc = %d\n", rc);
            exit(2);
        }
        break;
    case 'x':
        rc = search_and_replace(buff, BUFFER_SZ, user_str_len, argc, argv);
        if (rc < 0)
        {
            printf("Error searching and replacing, rc = %d\n", rc);
            exit(2);
        }
        break;
    default:
        usage(argv[0]);
        exit(1);
    }

    // TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);
    exit(0);
}

// TODO:  #7  Notice all of the helper functions provided in the
//           starter take both the buffer as well as the length.  Why
//           do you think providing both the pointer and the length
//           is a good practice, after all we know from main() that
//           the buff variable will have exactly 50 bytes?
//
//           PLACE YOUR ANSWER HERE
/* I would assume it would be good practice to help the programmer know how much memory had been allocated
 * If I didn't know the variable would have exactly 50 bytes, then the only way I would know I have reached
 * The maximum memory location allowed is through the length argument. It's good practice in case that were
 * to occur, to prevent the buffer overflow and attempting to access memory somewhere you are not allowed.
 */