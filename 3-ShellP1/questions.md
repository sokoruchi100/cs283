1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice for this application because it can store strings/char arrays line by line from the stdin. The shell is a line by line input program that uses the stdin and stdout to function. fgets allows the programmer to take the input from stdin and reads the entirety and stops once it encounters a newline character, which is when the user presses enter. This allows it to perform line by line functionality like processing the command. It also provides a safe way to set a max number of characters to read to prevent memory issues. 

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  I need to use malloc() to dynamically allocate heap memory to the pointer cmd_buff. A fixed size array can not grow in case we need to input more data. We can use malloc to to increase the size of the cmd_buff if necessary. Also, memory allocated on the heap persists and I will not have to worry about automatic deallocation. This means I will need to free the memory on my own. 


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  This is necessary because it might interpret commands including the spaces and it would not be able to run the commands with spaces in the name. It might also limit how many characters we can input in a single command. Ignoring the leading and trailing spaces allow us to input more characters. 

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  Input: prog1 < file1, Output: prog1 > file1, Append: prog1 >> file1. When doing pipes, we will have to 

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  _start here_

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  _start here_

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  _start here_