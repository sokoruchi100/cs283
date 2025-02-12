1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice for this application because it can store strings/char arrays line by line from the stdin. The shell is a line by line input program that uses the stdin and stdout to function. fgets allows the programmer to take the input from stdin and reads the entirety and stops once it encounters a newline character, which is when the user presses enter. This allows it to perform line by line functionality like processing the command. It also provides a safe way to set a max number of characters to read to prevent memory issues. gets() does not provide bounds checking and scanf() splits input on whitespace which makes it inaccurate to user input. 

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  I need to use malloc() to dynamically allocate heap memory to the pointer cmd_buff. A fixed size array can not grow in case we need to input more data. We can use malloc to to increase the size of the cmd_buff if necessary. For now, I set it at a size of SH_CMD_MAX, but this could potentially change if necessary.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  This is necessary because it might interpret commands including the spaces and it would not be able to run the commands with spaces in the name. You might run into errors like 'Command not found' or arguments might get messed up due to improper white spaces. Removing the whitespaces guarantees inputs are the same and increases reliability. 

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  Redirecting standard output to a file: ls > output.txt. Redirecting standard error to a file: ./badprog 2> error.txt. Redirecting input from a file: cat < file1. Some challenges include parsing the <, >, 2> and potentially more redirection characters. Each would need their own functionality, and it would have to be able to obtain the output from stdout/stderr and write that to a file. For input, it would need to read the file and then turn that into input for the given command.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection involves moving outputs to files or files to inputs for commands. They involve files while pipes to do not work with files. Instead, pipes work with other commands. Pipes take the output of one command and feeds it into the input of another command.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It is important to keep them separate so that programs and the user can distinguish between errors and normal outputs. You can redirect each stream differently, like one output.txt and another error.txt to check for only errors or only outputs. If they were all together, it will be much harder to differentiate.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  The custom shell should handle errors by reporting the exit status of failed commands. The commands themselves should handle reporting to stderr. If a command outputs to both stdout and stderr, we could merge them using 2>&1 which essentially redirects stderr to the file descriptor 1, which is stdout. This allows us to be flexible in our approach with handling stdout and stderr.