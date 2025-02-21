1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  If we called execvp directly, it would replace the current running process with whatever process was specified in the arguments. After the new process is finished running, it will never return back to the original process. In order to perform that functionality, we need to fork the process, which creates a child process. Then, we replace the child process with the new process image to perform the functionality while keeping the shell process.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork system call fails, I check to see if the returned value is less than 0. If it is, I return it from the exec_cmd function to the main loop function, where I break out of the loop. Then, I clean up the shell by freeing the memory and then I return back to main of dsh_cli.c, exiting the shell. It will return the error code for ERR_MEMORY.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  It finds the command by searching in the PATH environment variable. It uses the first argument to find the command in each directory listed in PATH until it can execute it, or it fails.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of wait() in the parent process is to stop the shell parent process and wait for the child to finish executing. This is to prevent a zombie process, which is a process that has finished executing, but the parent process has not yet recognized its completion. By calling wait, it will recognize the child's completion and reap it.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() extracts the exit status of the child process provided by wait(). This information is important because it indicates whether the executed command succeeded or encountered an error, allowing the parent process to react accordingly.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In my implementation, I create a new buffer to hold the token and iterate through the characters in the cmd_line. if the character is a quote, we treat everything that comes after it as part of the token, including the spaces, until another quote is reached. Afterwards, we continue to iterate normally if no quote is located. The iteration ends once we hit a space or the null terminator. This is necessary for handling arguments that contain spaces like file names or strings so they aren't split into separate tokens.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  I made a lot of changes with regards to parsing and cleanliness. I abstracted a lot of the code into smaller functions to make the code easier to read and understand what is ocurring to prevent bugs and make it easier to improve on in future assignments. As of now, I got rid of the pipe handling logic and instead, used a character iterator on the cmd_line string to build the cmd_buff_t struct. Most of the issues I encountered were handling double frees and avoiding memory leaks.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  The purpose of signals in a Linux system is to provide a mechanism for asynchronous notification to a process that an event as ocurred. They are different from other forms of IPC (like pipes) in that they do not transfer data, they only signal something has ocurred and the program can react to it or perform some default behavior.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL: forcefully terminates a process. It does not allow the process to perform any cleanup operations. The process does not have a chance to save its state or release resources and is only used as a last resort to terminate an unresponsive process, but it leaves the system in an inconsistent state. SIGTERM: Signals termination to a process, most likely sent by other processes rather than the user. The process terminates gracefully and also has a choice in how to handle the signal, like performing cleanup, saving state, or taking other actions before exiting. This is the preferred way to terminate a process. SIGINT: This is the signal sent to a process typically when pressing the Ctrl+C key combo. It requests a graceful termination to the process, so it will clean up any resources it has allocated. However, you can make programs that can ignore the SIGINT signal or even perform some kind of custom functionality.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receieves SIGSTOP, it is immediately suspended by the OS. Unlike SIGINT, it can not be caught or ignored like SIGINT because it is a reliable way to pause a running process regardless of its state. This makes sure an external entity like the system or a debugger can suspend the process when needed.