Contributions: 
Sushen: Echo, History, grep, ReadMe
Madhav: Pipes, background, Error Handling, and overall parsing of the code

github link: https://github.com/henten28/18-OS2

Limitations: 
Limited History Space: have only implemented an array of 100 
No Multiple Pipes:  shell supports only one pipe between two commands at a time
No Multiple Background's :one bg process is supported at a time
No cd/ changing directories
No custom signaling handling only the standard ones will mork. Things like sigmask and signals: too complex
no custom shell commands - as they will have to require their own definitions and there can be many of them


Explanation: So first under the main function we have
char *history[100] = {NULL};      // Array to store command history
pid_t pid_list[100] = {0};        // Array to store PIDs of executed commands
long time_list[100] = {0};        // Array to store execution times of commands
int zz = 0;                       // Counter to track the number of commands executed, this is used to store the index 
                                of the current processes pids and running time and their respective values are registered at this index.

now we will be using a do-while loop to run the SimpleShell
the user will input the command in the SimpleShell interface and as long as it doesnt exceed the history limit of 100 
the shell would run the command, by using the following fn: 

int create_process_and_run_single(char* args[], pid_t pid_list[], long time_list[], int zz) 
 //in this function command is broken down into a list of args which is then run using forking. 
 In the child process you run your command and the parent processes waits for the child process to run. 
 There was no need for forking but to maintan a generality of all the run processes functions this approach was chosen

we will also be noting down the time using gettimeofday(&start_time, NULL) and have done error handling for the same

If the user enters the history command, the shell loops through the history[] array and prints all previously entered commands with their index numbers.

If the user enters a command that is not a built-in command (history), SimpleShell attempts to execute it as an external system command.

Using fork:
shell creates a child process using fork() to execute the command. if fork() returns a negative value, it indicates that the creation of the child process failed- error handling
otherwise, we move on to the pipe and dothis again for the seconf Child

Child process:
In the child process (pid == 0), the command is executed using execvp(), which replaces the current process with the new command.
If the execution fails (e.g., the command is invalid), an error message is printed, and the child process exits with failure.


Parent Process:
The parent process waits for the child process to complete unless the command is intended to run in the bg ( by checking for  & ).
The parent process records the PID of the child process in pid_list[] and calculates the execution time and stores it in time_list[].


Pipe:
SimpleShell supports simple pipes (like ls | grep txt). The command is split on the pipe | symbol, and each part of the command is executed in sequence, passing the output of the first command as input to the second.

PID and Timing:
The PID of the executed command is recorded in the pid_list[]. 
The shell records the execution time in milliseconds from the start to the end of the command, and stores it in the time_list[].
