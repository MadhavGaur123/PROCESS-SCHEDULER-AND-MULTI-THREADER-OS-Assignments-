#include <stdio.h>      // Standard c library
#include <stdlib.h>     //  malloc, free ( used in elf also)
#include <string.h>     // strtok, strdup
#include <unistd.h>     // fork, execvp, pip
#include <sys/wait.h>   // Twaitpid 
#include <sys/time.h>   // gettimeofday
int create_process_and_run_single(char* args[], pid_t pid_list[], long time_list[], int zz) { //in this function command is broken down into a list of args which is then run using forking. In the child process you run your command and the parent processes waits for the child process to run. There was no need for forking but to maintan a generality of all the run processes functions this approach was chosen
    pid_t pid;              // Variable to store process 
    int status;             // Variable to store process status
    struct timeval start_time, end_time;  // this notes the time at which the program began running and notes the times when the process is finished. Its very similar to the python time library.

   //Process starts at this time
    gettimeofday(&start_time, NULL);  
    pid = fork();  
    if (pid < 0) {
        printf("Forking failed\n"); //error handling if forking failed
        return 0;
    } else if (pid == 0) {
        // Execute the command using execvp, which replaces the current process image with a new one. In the child process the actual argument is run
        if (execvp(args[0], args) == -1) {
            perror("Error executing command");
        }
        printf("Error regarding execvp");
        return 0;// If execvp fails, give error
    } else { // The parent process does nothing apart from waiting for the child process to finish and then noting some of its features in a lists for the print history function.
      // Store the child PID in pid_list
        pid_list[zz] = pid;  
        // Wait for the child process to finish
        waitpid(pid, &status, WUNTRACED);

        //  end time when the process finished execution
        gettimeofday(&end_time, NULL); 

        // general formula to calculate time diff( end time - start time ) with millisecond precision
        time_list[zz] = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_usec - start_time.tv_usec);  
    }
    return 1;  // returning 1 to imply successfull implementation
}

// Function to create two processes with a pipe between them (for commands with pipes)
// It stores the PID of the first process in pid_list and the time taken to execute the entire pipeline in time_list
int create_process_and_run(char* args1[], char* args2[], pid_t pid_list[], long time_list[], int zz) {
    pid_t pid1, pid2;    //  store PIDs of two processes
    int fd[2];           // File descriptors for the pipe fd[0] - The read end of the pipeling and fd[1] the write end of the pipeline
    int status;          // Variable to store process status
    struct timeval start_time, end_time;  // To track the start and end times of the pipeline execution

    //Process starts at this time
    gettimeofday(&start_time, NULL);  

    // Create a pipe (fd[0] for reading, fd[1] for writing)
    if (pipe(fd) == -1) {
        perror("Pipe failed"); // error handling if file descriptor of the fd is -1
        return 0;
    }
    // Fork the first child process to run the first command
    pid1 = fork();
    if (pid1 < 0) {
        perror("Forking first child failed");
        return 0;
    } else if (pid1 == 0) {
        // Redirect stdout to the pipe's write end
        dup2(fd[1], STDOUT_FILENO);  // this function writes/ dumps the stout of the command running in the write end of the pipeline.
        close(fd[0]);  // Close the read end of the pipe
        close(fd[1]);  // Close the write end of the pipe after redirection
        // the first command is run in the pipeline
        if (execvp(args1[0], args1) == -1) {// error handling if execvp fails
            perror("Error executing first command");
            return 0;
        }
          
    }

    // Fork the second child process to run the second command
    pid2 = fork();
    if (pid2 < 0) {
        perror("Forking second child failed");
        return 0;
    } else if (pid2 == 0) {
        // Redirect stdin to the pipe's read end
        dup2(fd[0], STDIN_FILENO);  // this function reads/ dumps the stout of the command onto the read end of the pipeline.
        close(fd[1]);  // Close the write end of the pipe
        close(fd[0]);  // Close the read end of the pipe after redirection
        // Execute the second command
        if (execvp(args2[0], args2) == -1) {
            perror("Error executing second command");// error handling if execvp faails
            return 0;
        }
        
    }
    
    close(fd[0]);//close the entire pipeline
    close(fd[1]);
    waitpid(pid1, &status, 0);  // wait for both the processes to finish
    waitpid(pid2, &status, 0);
    // the time at which the processes ends
    gettimeofday(&end_time, NULL);  

    // Store the PID of the first process as that will imply the entire process id of the entire pipeling function
    pid_list[zz] = pid1;  

    // general formula to calculate time diff( end time - start time ) with millisecond precision
    time_list[zz] = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_usec - start_time.tv_usec);  
    return 1;
}

// Main launch functions which maps which type of function will run which type of command
int launch(char* command, char* history[], pid_t pid_list[], long time_list[], int zz) {
    char *token;
    char *args[100];  //when the commandline argument is split and stored in the args array
    int i = 0;
    // traverse through each word in args and split it further
    token = strtok(command, " \n");
    
    // Check if the command is "history", which shows the list of executed commands
    if (strcmp(token, "history") == 0) {
        // Display history, PID, and time for each command
        for (i = 0; history[i] != NULL; i++) {
            printf("%d: %s", i + 1, history[i]);
            printf("   PID: %d\n", pid_list[i]);
            printf("   Duration: %ld microseconds\n", time_list[i]);
            printf("\n");
        }
        return 1;  // Return 1 to continue the shell loop
    }

    // Parse the command into arguments
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;  // Null-terminate the argument list, so that all the loops work properly. As if an element has 10 enteries we assign those 10 values and make all the others null so that traversing through the array is easier.

    // If no arguments were provided (empty command), continue the shell loop
    if (args[0] == NULL) {
        return 1;
    }

    // Variables to store the index of pipe "|" and background "&" symbols
    int pipe_index = -1;
    int background_index = -1;

    // Check if the command contains a pipe ("|")
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }

    // Check if the command should run in the background ("&")
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            background_index = i;
            break;
        }
    }

    // If no pipe is found
    if (pipe_index == -1) {
        // If no background symbol is found, run the single command
        if (background_index == -1) {
            return create_process_and_run_single(args, pid_list, time_list, zz);
        } else {
            // If a background symbol is found, run two commands separately
            args[background_index] = NULL;  
            char **args1 = args;  
            char **args2 = &args[background_index + 1]; 
            int k = create_process_and_run_single(args1, pid_list, time_list, zz);
            int K = create_process_and_run_single(args2, pid_list, time_list, zz);
            return (k && K);
        }
    } else {
        // If a pipe is found, split the arguments into two commands
        args[pipe_index] = NULL; 
        char **args1 = args;  
        char **args2 = &args[pipe_index + 1];  
        return create_process_and_run(args1, args2, pid_list, time_list, zz); 
    }

    return 0;
}

// Function to read user input from the command line
// this code in in sirs lecture notes i have taken reference from there as it is
char* read_user_input() {
    char *line = NULL;
    size_t bufsize = 0;

    // getline dynamically allocates memory and reads a line from stdin
    getline(&line, &bufsize, stdin);  
    return line;
}
// this code in in sirs lecture notes i have taken reference from there as it is
int main(int argc, char *argv[]) {
    char *history[100] = {NULL};      // Array to store command history
    pid_t pid_list[100] = {0};        // Array to store PIDs of executed commands
    long time_list[100] = {0};        // Array to store execution times of commands
    int zz = 0;                       // Counter to track the number of commands executed, this is used to store the index of the current processes pids and running time and their respective values are registered at this index.
    int status;

    do {
        // Print the shell prompt
        printf("SimpleShell>>> ");  
        
        // Read the user input
        char* command = read_user_input(); 

        // Store the command in history (up to 100 commands)
        if (zz < 100) {
            history[zz] = strdup(command);  // Store command in history
        } else {
            perror("History overflow"); //i have assumed that the history array can only store the history of 100 commands if you keep on running the while loop for more that 100 commands the array will get full and an error will be displayed
            return 0;
        }

        // Launch the command and pass history, pid_list, and time_list
        status = launch(command, history, pid_list, time_list, zz);  

        // Increment the command counter to keep track of the processes
        zz = zz + 1;

        // Free the dynamically allocated memory for the command
        // to handle memory properly
        free(command);  
    } while (status);  //infinite while loop for the interface
    return 0;
}


