1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: We use fork() before execvp() because fork() creates a new process (a child process), so our shell program can keep running while the command executes. If we called execvp() directly, it would replace our shell process, and we wouldn't be able to take new commands after the first one runs. fork() makes sure the shell stays alive.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If fork() fails, it means the system couldn’t create a new process, usually due to resource exhaustion (too many processes running). In my implementation, if fork() returns -1, an error message is printed using perror("fork failed"), and the shell does not attempt to execute the command. Instead, it returns to the prompt to allow further input.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: execvp() looks for the command in the directories listed in the PATH environment variable. PATH is a system setting that tells where to find programs. If a command like ls is entered, execvp() checks folders like /usr/bin/ and /bin/ to find and run ls. If the command isn't found in PATH, it fails.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**: The wait() function makes the parent process (the shell) pause until the child process (the command execution) finishes. If we don’t call wait(), the child process would consume system resources unnecessarily. It also helps the shell retrieve the exit status of the child process to report errors or success.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS(status) gives us the exit code of the command that just ran. This helps the shell know if the command ran successfully or if it failed. A return code of 0 usually means success, while a non-zero value means there was an error.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My build_cmd_buff() function makes sure that words inside quotes are treated as one argument instead of being split into multiple arguments. It does this by using a variable in_quotes to track if we are inside quotes. This is necessary because if a user types: echo "Hello World", without handling quotes, it would treat "Hello" and "World" as separate arguments, but we want them to be passed as one.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: The new version improves: handling of quoted arguments (so "Hello World" stays together), trimming spaces better, more careful memory management to avoid crashes. One challenge in refactoring was making sure multiple spaces, quotes, and special characters are handled correctly while keeping the code simple.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are a way for the system to send messages to processes. Unlike other IPC methods (like shared memory or message queues), signals are simple and quick. They can be used to stop, pause, or kill a process immediately.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGINT (2) – Sent when you press Ctrl+C, stops a program (e.g., stops a running script).
    > **Answer**:  SIGTERM (15) – Asks a process to gracefully shut down (e.g., stopping a server safely).
    > **Answer**:  SIGKILL (9) – Forces a process to stop immediately (e.g., when a program is stuck and won’t close).

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process gets SIGSTOP, it pauses completely. It won’t run again until it gets SIGCONT. Unlike SIGINT, it cannot be caught or ignored because the system forces the process to stop no matter what. This is useful when we need to pause a program and later resume it, like using Ctrl+Z in a terminal.
