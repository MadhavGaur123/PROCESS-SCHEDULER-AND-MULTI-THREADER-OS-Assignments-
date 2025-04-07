// Importing various libraries
#include <stdio.h>    // Standard Input and Output Library for functions like printf and scanf
#include <stdlib.h>   // Standard Library for memory allocation, process control, conversions, etc.
#include <unistd.h>   // Provides access to the POSIX operating system API (e.g., for working with processes)
#include <sys/wait.h> // Definitions for waiting for process termination
#include <time.h>     // Time handling functions for date and time manipulation
#include <signal.h>   // Definitions for handling signals in the program (e.g., SIGINT, SIGTERM)
#include <semaphore.h> // POSIX semaphore functions for managing concurrent access to resources
#include <string.h>   // String handling functions for manipulating C strings (e.g., strcpy, strlen)
#include <fcntl.h>    // File control options (e.g., open, creat) for file descriptor manipulation
#include <sys/mman.h> // Memory management declarations for shared memory and memory mapping
#include <sys/time.h> // Time-related functions, including timers and time measurement
#include <errno.h>    // Macro for reporting error numbers
#include <stdbool.h>  // Boolean type and values (true, false) for better readability


#define MAX_JOBS 100
#define SHM_NAME "/job_array"

typedef struct {
    pid_t pid;                     // Process ID of the job
    int is_running;                // Flag indicating if the job is currently running
    int in_queue;                  // Flag indicating if the job is in the queue
    long long submission_time;     // Time when the job was submitted
    long long last_start_time;     // Time when the job last started
    long long wait_time;           // Total wait time of the job
    long long run_time;            // Total run time of the job
    long long completion_time;      // Time when the job was completed
} Job;

// Structure to hold shared state information
typedef struct {
    int terminate;     // Flag to indicate termination request
    int job_count;     // Count of jobs in priority 1
    int job_count2;    // Count of jobs in priority 2
    int job_count3;    // Count of jobs in priority 3
    int job_count4;    // Count of jobs in priority 4
} SharedState;

Job *jobs;
Job *jobs2;
Job *jobs3;
Job *jobs4;
SharedState *shared_state;
sem_t *sem_job_state;




long long current_time_ms() { // To get the time at the moment you call the function
 struct timeval te;
 gettimeofday(&te, NULL);
 return te.tv_sec * 1000LL + te.tv_usec / 1000;
}

//Function to submit 1 job
void submit_job(char *args[], int priority, int TSLICE) {
 pid_t pid; // pid to fork in the child run the program in the parent create a job type of node in the array initialise its contents and then stop the child program using kill. This will now enable us to pause and run the program at our own will in the scheduler


 sem_wait(sem_job_state); // semaphores to avoid race condition
// Based on the priority entered the job will be submitted in its respective queue. I have used switch case syntax rather than if else to do the above
 switch(priority) {
 case 1:
 if (shared_state->job_count >= MAX_JOBS) {
 printf("Job queue for priority 1 is full!\n"); // error handling
 sem_post(sem_job_state);
 return;
 }
 pid = fork();
 if (pid < 0) {
 perror("Forking failed"); // error handling
 sem_post(sem_job_state);
 return;
 }
 else if (pid == 0) {
 
 execvp(args[0], args); // running the program in the child
 perror("Error executing job");
 //sem_post(sem_job_state);
 exit(1);
 }
 else { // Initialising parameters
 jobs[shared_state->job_count].pid = pid;
 jobs[shared_state->job_count].is_running = 1;
 jobs[shared_state->job_count].in_queue = 1;
 jobs[shared_state->job_count].submission_time = current_time_ms();
 jobs[shared_state->job_count].last_start_time = 0;
 jobs[shared_state->job_count].wait_time = 0;
 jobs[shared_state->job_count].run_time = 0;
 jobs[shared_state->job_count].completion_time = 0;
 shared_state->job_count++;
 kill(pid, SIGSTOP); // killing the program so that we can use round robin
 printf("Job submitted: %s with PID %d\n", args[0], pid);
 sem_post(sem_job_state);
 }
 break;

 case 2: // this also works in the same principle as the above
 if (shared_state->job_count2 >= MAX_JOBS) {
 printf("Job queue for priority 2 is full!\n");
 sem_post(sem_job_state);
 return;
 }
 pid = fork();
 if (pid < 0) {
 perror("Forking failed");
 sem_post(sem_job_state);
 return;
 } else if (pid == 0) {
 
 execvp(args[0], args);
 perror("Error executing job");
 sem_post(sem_job_state);
 exit(1);
 } else {
 jobs2[shared_state->job_count2].pid = pid;
 jobs2[shared_state->job_count2].is_running = 1;
 jobs2[shared_state->job_count2].in_queue = 1;
 jobs2[shared_state->job_count2].submission_time = current_time_ms();
 jobs2[shared_state->job_count2].last_start_time = 0;
 jobs2[shared_state->job_count2].wait_time = 0;
 jobs2[shared_state->job_count2].run_time = 0;
 jobs2[shared_state->job_count2].completion_time = 0;
 shared_state->job_count2++;
 kill(pid, SIGSTOP);
 printf("Job submitted: %s with PID %d\n", args[0], pid);
 sem_post(sem_job_state);
 }
 break;

 case 3:
 if (shared_state->job_count3 >= MAX_JOBS) {
 printf("Job queue for priority 3 is full!\n");
 sem_post(sem_job_state);
 return;
 }
 pid = fork();
 if (pid < 0) {
 perror("Forking failed");
 sem_post(sem_job_state);
 return;
 } else if (pid == 0) {
 
 execvp(args[0], args);
 sem_post(sem_job_state);
 perror("Error executing job");
 exit(1);
 } else {
 jobs3[shared_state->job_count3].pid = pid;
 jobs3[shared_state->job_count3].is_running = 1;
 jobs3[shared_state->job_count3].in_queue = 1;
 jobs3[shared_state->job_count3].submission_time = current_time_ms();
 jobs3[shared_state->job_count3].last_start_time = 0;
 jobs3[shared_state->job_count3].wait_time = 0;
 jobs3[shared_state->job_count3].run_time = 0;
 jobs3[shared_state->job_count3].completion_time = 0;
 shared_state->job_count3++;
 kill(pid, SIGSTOP);
 sem_post(sem_job_state);
 printf("Job submitted: %s with PID %d\n", args[0], pid);
 }
 break;

 case 4:
 if (shared_state->job_count4 >= MAX_JOBS) {
 printf("Job queue for priority 4 is full!\n");
 sem_post(sem_job_state);
 return;
 }
 pid = fork();
 if (pid < 0) {
 perror("Forking failed");
 sem_post(sem_job_state);
 return;
 } else if (pid == 0) {
 execvp(args[0], args);
 sem_post(sem_job_state);
 perror("Error executing job");
 exit(1);
 } else {
 jobs4[shared_state->job_count4].pid = pid;
 jobs4[shared_state->job_count4].is_running = 1;
 jobs4[shared_state->job_count4].in_queue = 1;
 jobs4[shared_state->job_count4].submission_time = current_time_ms();
 jobs4[shared_state->job_count4].last_start_time = 0;
 jobs4[shared_state->job_count4].wait_time = 0;
 jobs4[shared_state->job_count4].run_time = 0;
 jobs4[shared_state->job_count4].completion_time = 0;
 shared_state->job_count4++;
 kill(pid, SIGSTOP);
 sem_post(sem_job_state);
 printf("Job submitted: %s with PID %d\n", args[0], pid);
 }
 break;

 default:
 printf("Invalid priority: %d\n", priority);
 sem_post(sem_job_state);
 return;
 }

 printf("Current jobs in queue:\n");
 for (int i = 0; i < shared_state->job_count; i++) {
 if (jobs[i].in_queue == 1) {
 printf("Priority 1 Job PID: %d\n", jobs[i].pid);
 }
 }
 for (int i = 0; i < shared_state->job_count2; i++) {
 if (jobs2[i].in_queue == 1) {
 printf("Priority 2 Job PID: %d\n", jobs2[i].pid);
 }
 }
 for (int i = 0; i < shared_state->job_count3; i++) {
 if (jobs3[i].in_queue == 1) {
 printf("Priority 3 Job PID: %d\n", jobs3[i].pid);
 }
 }
 for (int i = 0; i < shared_state->job_count4; i++) {
 if (jobs4[i].in_queue == 1) {
 printf("Priority 4 Job PID: %d\n", jobs4[i].pid);
 }
 }
 sem_post(sem_job_state);
}

// This code is for submitting two args . Basically it works on the same principle as the above
void submit_job_2(char *args1[], int priority1, char *args2[], int priority2, int TSLICE) {
    pid_t pid;

    sem_wait(sem_job_state);

    // Submit the first job with args1 and priority1
    switch(priority1) {
        case 1:
            if (shared_state->job_count >= MAX_JOBS) {
                printf("Job queue for priority 1 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args1[0], args1);
                perror("Error executing job");
                exit(1);
            } else {
                jobs[shared_state->job_count].pid = pid;
                jobs[shared_state->job_count].is_running = 1;
                jobs[shared_state->job_count].in_queue = 1;
                jobs[shared_state->job_count].submission_time = current_time_ms();
                jobs[shared_state->job_count].last_start_time = 0;
                jobs[shared_state->job_count].wait_time = 0;
                jobs[shared_state->job_count].run_time = 0;
                jobs[shared_state->job_count].completion_time = 0;
                shared_state->job_count++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args1[0], pid);
            }
            break;

        case 2:
            if (shared_state->job_count2 >= MAX_JOBS) {
                printf("Job queue for priority 2 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args1[0], args1);
                perror("Error executing job");
                exit(1);
            } else {
                jobs2[shared_state->job_count2].pid = pid;
                jobs2[shared_state->job_count2].is_running = 1;
                jobs2[shared_state->job_count2].in_queue = 1;
                jobs2[shared_state->job_count2].submission_time = current_time_ms();
                jobs2[shared_state->job_count2].last_start_time = 0;
                jobs2[shared_state->job_count2].wait_time = 0;
                jobs2[shared_state->job_count2].run_time = 0;
                jobs2[shared_state->job_count2].completion_time = 0;
                shared_state->job_count2++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args1[0], pid);
            }
            break;

        case 3:
            if (shared_state->job_count3 >= MAX_JOBS) {
                printf("Job queue for priority 3 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args1[0], args1);
                perror("Error executing job");
                exit(1);
            } else {
                jobs3[shared_state->job_count3].pid = pid;
                jobs3[shared_state->job_count3].is_running = 1;
                jobs3[shared_state->job_count3].in_queue = 1;
                jobs3[shared_state->job_count3].submission_time = current_time_ms();
                jobs3[shared_state->job_count3].last_start_time = 0;
                jobs3[shared_state->job_count3].wait_time = 0;
                jobs3[shared_state->job_count3].run_time = 0;
                jobs3[shared_state->job_count3].completion_time = 0;
                shared_state->job_count3++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args1[0], pid);
            }
            break;

        case 4:
            if (shared_state->job_count4 >= MAX_JOBS) {
                printf("Job queue for priority 4 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args1[0], args1);
                perror("Error executing job");
                exit(1);
            } else {
                jobs4[shared_state->job_count4].pid = pid;
                jobs4[shared_state->job_count4].is_running = 1;
                jobs4[shared_state->job_count4].in_queue = 1;
                jobs4[shared_state->job_count4].submission_time = current_time_ms();
                jobs4[shared_state->job_count4].last_start_time = 0;
                jobs4[shared_state->job_count4].wait_time = 0;
                jobs4[shared_state->job_count4].run_time = 0;
                jobs4[shared_state->job_count4].completion_time = 0;
                shared_state->job_count4++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args1[0], pid);
            }
            break;

        default:
            printf("Invalid priority: %d\n", priority1);
            sem_post(sem_job_state);
            return;
    }

    // Now submit the second job with args2 and priority2
    switch(priority2) {
        case 1:
            if (shared_state->job_count >= MAX_JOBS) {
                printf("Job queue for priority 1 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args2[0], args2);
                perror("Error executing job");
                exit(1);
            } else {
                jobs[shared_state->job_count].pid = pid;
                jobs[shared_state->job_count].is_running = 1;
                jobs[shared_state->job_count].in_queue = 1;
                jobs[shared_state->job_count].submission_time = current_time_ms();
                jobs[shared_state->job_count].last_start_time = 0;
                jobs[shared_state->job_count].wait_time = 0;
                jobs[shared_state->job_count].run_time = 0;
                jobs[shared_state->job_count].completion_time = 0;
                shared_state->job_count++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args2[0], pid);
            }
            break;

        case 2:
            if (shared_state->job_count2 >= MAX_JOBS) {
                printf("Job queue for priority 2 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args2[0], args2);
                perror("Error executing job");
                exit(1);
            } else {
                jobs2[shared_state->job_count2].pid = pid;
                jobs2[shared_state->job_count2].is_running = 1;
                jobs2[shared_state->job_count2].in_queue = 1;
                jobs2[shared_state->job_count2].submission_time = current_time_ms();
                jobs2[shared_state->job_count2].last_start_time = 0;
                jobs2[shared_state->job_count2].wait_time = 0;
                jobs2[shared_state->job_count2].run_time = 0;
                jobs2[shared_state->job_count2].completion_time = 0;
                shared_state->job_count2++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args2[0], pid);
            }
            break;

        case 3:
            if (shared_state->job_count3 >= MAX_JOBS) {
                printf("Job queue for priority 3 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args2[0], args2);
                perror("Error executing job");
                exit(1);
            } else {
                jobs3[shared_state->job_count3].pid = pid;
                jobs3[shared_state->job_count3].is_running = 1;
                jobs3[shared_state->job_count3].in_queue = 1;
                jobs3[shared_state->job_count3].submission_time = current_time_ms();
                jobs3[shared_state->job_count3].last_start_time = 0;
                jobs3[shared_state->job_count3].wait_time = 0;
                jobs3[shared_state->job_count3].run_time = 0;
                jobs3[shared_state->job_count3].completion_time = 0;
                shared_state->job_count3++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args2[0], pid);
            }
            break;

        case 4:
            if (shared_state->job_count4 >= MAX_JOBS) {
                printf("Job queue for priority 4 is full!\n");
                sem_post(sem_job_state);
                return;
            }
            pid = fork();
            if (pid < 0) {
                perror("Forking failed");
                sem_post(sem_job_state);
                return;
            } else if (pid == 0) {
                execvp(args2[0], args2);
                perror("Error executing job");
                exit(1);
            } else {
                jobs4[shared_state->job_count4].pid = pid;
                jobs4[shared_state->job_count4].is_running = 1;
                jobs4[shared_state->job_count4].in_queue = 1;
                jobs4[shared_state->job_count4].submission_time = current_time_ms();
                jobs4[shared_state->job_count4].last_start_time = 0;
                jobs4[shared_state->job_count4].wait_time = 0;
                jobs4[shared_state->job_count4].run_time = 0;
                jobs4[shared_state->job_count4].completion_time = 0;
                shared_state->job_count4++;
                kill(pid, SIGSTOP);
                printf("Job submitted: %s with PID %d\n", args2[0], pid);
            }
            break;

        default:
            printf("Invalid priority: %d\n", priority2);
            sem_post(sem_job_state);
            return;
    }

    // Print the ready queue for both priorities. Basically to maintain the ready queue
    printf("Current jobs in queue:\n");
    for (int i = 0; i < shared_state->job_count; i++) {
        if (jobs[i].in_queue == 1) {
            printf("Priority 1 Job PID: %d\n", jobs[i].pid);
        }
    }
    for (int i = 0; i < shared_state->job_count2; i++) {
        if (jobs2[i].in_queue == 1) {
            printf("Priority 2 Job PID: %d\n", jobs2[i].pid);
        }
    }
    for (int i = 0; i < shared_state->job_count3; i++) {
        if (jobs3[i].in_queue == 1) {
            printf("Priority 3 Job PID: %d\n", jobs3[i].pid);
        }
    }
    for (int i = 0; i < shared_state->job_count4; i++) {
        if (jobs4[i].in_queue == 1) {
            printf("Priority 4 Job PID: %d\n", jobs4[i].pid);
        }
    }
    sem_post(sem_job_state); // release semaphores
}



void print_stats(int TSLICE) {
    // Wait for the semaphore to ensure exclusive access to job stats
    sem_wait(sem_job_state);
    
    // Print header for the job completion statistics
    printf("\n--- Job Completion Stats ---\n");
    
    // Iterate through the jobs in priority 1
    for (int i = 0; i < shared_state->job_count; i++) {
        // Check if the job has completed (not in the queue)
        if (jobs[i].in_queue == 0) {
            // Print details of the completed job including PID, wait time, run time, and completion time
            printf("Job PID: %d, Wait Time: %lld ms, Run Time: %lld ms, Completion Time: %lld ms\n",
                   jobs[i].pid, jobs[i].wait_time, jobs[i].run_time,
                   jobs[i].completion_time);
        }
    }

    // Iterate through the jobs in priority 2
    for (int i = 0; i < shared_state->job_count2; i++) {
        // Check if the job has completed (not in the queue)
        if (jobs2[i].in_queue == 0) {
            // Print details of the completed job
            printf("Job PID: %d, Wait Time: %lld ms, Run Time: %lld ms, Completion Time: %lld ms\n",
                   jobs2[i].pid, jobs2[i].wait_time, jobs2[i].run_time,
                   jobs2[i].completion_time);
        }
    }

    // Iterate through the jobs in priority 3
    for (int i = 0; i < shared_state->job_count3; i++) {
        // Check if the job has completed (not in the queue)
        if (jobs3[i].in_queue == 0) {
            // Print details of the completed job
            printf("Job PID: %d, Wait Time: %lld ms, Run Time: %lld ms, Completion Time: %lld ms\n",
                   jobs3[i].pid, jobs3[i].wait_time, jobs3[i].run_time,
                   jobs3[i].completion_time);
        }
    }

    // Iterate through the jobs in priority 4
    for (int i = 0; i < shared_state->job_count4; i++) {
        // Check if the job has completed (not in the queue)
        if (jobs4[i].in_queue == 0) {
            // Print details of the completed job
            printf("Job PID: %d, Wait Time: %lld ms, Run Time: %lld ms, Completion Time: %lld ms\n",
                   jobs4[i].pid, jobs4[i].wait_time, jobs4[i].run_time,
                   jobs4[i].completion_time);
        }
    }

    // Release the semaphore to allow other processes to access job stats
    sem_post(sem_job_state);
}
// Function to handle the submission of a single job
void handle_job_submission(char *args[], int TSLICE) {
    // Check if the command starts with "submit" and has a specified priority argument
    if (strcmp(args[0], "submit") != 0 || args[2] == NULL) {
        // If the format is invalid, print an error message and exit the function
        printf("Invalid command format. The command must start with 'submit' and include a priority.\n");
        return;
    }

    // Convert the priority argument from string to integer
    int priority = atoi(args[2]);
    
    // Validate that the priority is within the accepted range (1 to 4)
    if (priority < 1 || priority > 4) {
        // If the priority is invalid, print an error message and exit the function
        printf("Invalid priority number: %d. Priority must be between 1 and 4.\n", priority);
        return;
    }

    // Call the function to submit the job with the arguments and priority
    submit_job(args + 1, priority, TSLICE);
}

// Function to handle the submission of two jobs simultaneously
void handle_job_submission_2(char *args[], char *args2[], int TSLICE) {
    // Check if the first command starts with "submit" and has a specified priority argument
    if (strcmp(args[0], "submit") != 0 || args[2] == NULL) {
        // If the format is invalid, print an error message and exit the function
        printf("Invalid command format. The command must start with 'submit' and include a priority.\n");
        return;
    }

    // Convert the priority argument from string to integer for the first job
    int priority1 = atoi(args[2]);
    
    // Validate that the priority for the first job is within the accepted range (1 to 4)
    if (priority1 < 1 || priority1 > 4) {
        // If the priority is invalid, print an error message and exit the function
        printf("Invalid priority number: %d. Priority must be between 1 and 4.\n", priority1);
        return;
    }

    // Check if the second command starts with "submit" and has a specified priority argument
    if (strcmp(args2[0], "submit") != 0 || args2[2] == NULL) {
        // If the format is invalid, print an error message and exit the function
        printf("Invalid command format. The command must start with 'submit' and include a priority.\n");
        return;
    }

    // Convert the priority argument from string to integer for the second job
    int priority2 = atoi(args2[2]);
    
    // Validate that the priority for the second job is within the accepted range (1 to 4)
    if (priority2 < 1 || priority2 > 4) {
        // If the priority is invalid, print an error message and exit the function
        printf("Invalid priority number: %d. Priority must be between 1 and 4.\n", priority2);
        return;
    }

    // Call the function to submit the two jobs with their respective arguments and priorities
    submit_job_2(args + 1, priority1, args2 + 1, priority2, TSLICE);
}



int is_process_terminated(pid_t pid) {
    // Try sending signal 0 to check if the process exists
    if (kill(pid, 0) == 0) {
        // Process exists
        return 0; // Process is not terminated
    } else {
        // Process does not exist or we don't have permission
        if (errno == ESRCH) {
            // ESRCH means the process does not exist
            return 1; // Process has terminated
        } else if (errno == EPERM) {
            // EPERM means the process exists but we don't have permission
            return 0; // Process is not terminated
        }
    }
    return 1; // Return 1 to be safe if there's an unknown error
}
void termination_request(int signum) { // To handle ctrl+c signal request
 (void)signum;
 shared_state->terminate = 1;
 
 printf("Scheduler exiting...\n");
 printf("Shell exiting... \n");
 exit(1);
}

// Function to manage job scheduling based on Round Robin strategy
void scheduler(int NCPU, int TSLICE) {
    // Loop until the shared state signals termination
    while (!shared_state->terminate) {
        // Wait for access to the shared job state semaphore
        sem_wait(sem_job_state);
        
        int num_invoked = 0; // Counter for the number of invoked jobs
        int num[NCPU][2];    // Array to store job index and its corresponding queue
        
        // Process jobs in the 'jobs' array
        for (int i = 0; i < shared_state->job_count; i++) {
            if (jobs[i].in_queue == 1) { // Check if the job is currently in the queue
                // If there are available CPUs, invoke the job
                if (num_invoked < NCPU) {
                    num[num_invoked][0] = i;    // Store the index of the job
                    num[num_invoked][1] = 1;    // Indicates the job belongs to the 'jobs' array
                    kill(jobs[i].pid, SIGCONT);  // Continue the job process
                    num_invoked++;               // Increment the count of invoked jobs
                    jobs[i].run_time += TSLICE;  // Update the run time of the job
                } else {
                    // If no CPUs are available, increment the wait time of the job
                    jobs[i].wait_time += TSLICE;
                }
            }
        }
        
        // Process jobs in the 'jobs2' array
        for (int i = 0; i < shared_state->job_count2; i++) {
            if (jobs2[i].in_queue == 1) { // Check if the job is currently in the queue
                if (num_invoked < NCPU) {
                    num[num_invoked][0] = i;    // Store the index of the job
                    num[num_invoked][1] = 2;    // Indicates the job belongs to the 'jobs2' array
                    kill(jobs2[i].pid, SIGCONT); // Continue the job process
                    num_invoked++;               // Increment the count of invoked jobs
                    jobs2[i].run_time += TSLICE; // Update the run time of the job
                } else {
                    // If no CPUs are available, increment the wait time of the job
                    jobs2[i].wait_time += TSLICE;
                }
            }
        }

        // Process jobs in the 'jobs3' array
        for (int i = 0; i < shared_state->job_count3; i++) {
            if (jobs3[i].in_queue == 1) { // Check if the job is currently in the queue
                if (num_invoked < NCPU) {
                    num[num_invoked][0] = i;    // Store the index of the job
                    num[num_invoked][1] = 3;    // Indicates the job belongs to the 'jobs3' array
                    kill(jobs3[i].pid, SIGCONT); // Continue the job process
                    num_invoked++;               // Increment the count of invoked jobs
                    jobs3[i].run_time += TSLICE; // Update the run time of the job
                } else {
                    // If no CPUs are available, increment the wait time of the job
                    jobs3[i].wait_time += TSLICE;
                }
            }
        }

        // Process jobs in the 'jobs4' array
        for (int i = 0; i < shared_state->job_count4; i++) {
            if (jobs4[i].in_queue == 1) { // Check if the job is currently in the queue
                if (num_invoked < NCPU) {
                    num[num_invoked][0] = i;    // Store the index of the job
                    num[num_invoked][1] = 4;    // Indicates the job belongs to the 'jobs4' array
                    kill(jobs4[i].pid, SIGCONT); // Continue the job process
                    num_invoked++;               // Increment the count of invoked jobs
                    jobs4[i].run_time += TSLICE; // Update the run time of the job
                } else {
                    // If no CPUs are available, increment the wait time of the job
                    jobs4[i].wait_time += TSLICE;
                }
            }
        }
        
        // Release the semaphore after processing jobs
        sem_post(sem_job_state);
        usleep(TSLICE); // Sleep for TSLICE milliseconds to allow job processing time

        // Process each invoked job based on the queue number in `num`
        for (int i = 0; i < num_invoked; i++) {
            int job_index = num[i][0];    // Get the job index
            int job_queue = num[i][1];     // Get the job queue number

            // Handle job termination and completion for the 'jobs' array
            if (job_queue == 1) {
                printf("%d\n", jobs[job_index].pid); // Print the job PID
                int check = is_process_terminated(jobs[job_index].pid); // Check if the job has terminated
                kill(jobs[job_index].pid, SIGSTOP); // Stop the job process
                if (check == 0) { // If the job is still running
                    sem_wait(sem_job_state); // Wait for access to shared state
                    jobs[job_index].in_queue = 0; // Mark the job as no longer in queue
                    jobs[job_index].completion_time = jobs[job_index].run_time + jobs[job_index].wait_time ; // Calculate completion time
                    sem_post(sem_job_state); // Release the shared state semaphore
                }
                printf("%d\n", jobs[job_index].in_queue); // Print the job's in_queue status
            } 
            // Similar logic is applied for jobs in the 'jobs2', 'jobs3', and 'jobs4' arrays
            else if (job_queue == 2) {
                printf("%d\n", jobs2[job_index].pid);
                int check = is_process_terminated(jobs2[job_index].pid);
                kill(jobs2[job_index].pid, SIGSTOP);
                if (check == 0) {
                    sem_wait(sem_job_state);
                    jobs2[job_index].in_queue = 0;
                    jobs2[job_index].completion_time = jobs2[job_index].run_time + jobs2[job_index].wait_time ;
                    sem_post(sem_job_state);
                }
                printf("%d\n", jobs2[job_index].in_queue);
            } else if (job_queue == 3) {
                printf("%d\n", jobs3[job_index].pid);
                int check = is_process_terminated(jobs3[job_index].pid);
                kill(jobs3[job_index].pid, SIGSTOP);
                if (check == 0) {
                    sem_wait(sem_job_state);
                    jobs3[job_index].in_queue = 0;
                    jobs3[job_index].completion_time =jobs3[job_index].run_time + jobs3[job_index].wait_time ;
                    sem_post(sem_job_state);
                }
                printf("%d\n", jobs3[job_index].in_queue);
            } else if (job_queue == 4) {
                printf("%d\n", jobs4[job_index].pid);
                int check = is_process_terminated(jobs4[job_index].pid);
                kill(jobs4[job_index].pid, SIGSTOP);
                if (check == 0) {
                    sem_wait(sem_job_state);
                    jobs4[job_index].in_queue = 0;
                    jobs4[job_index].completion_time = jobs4[job_index].run_time + jobs4[job_index].wait_time ;
                    sem_post(sem_job_state);
                }
                printf("%d\n", jobs4[job_index].in_queue);
            }
        }
    }
}



// Function to implement a simple shell interface for submitting jobs
void simple_shell(int NCPU, int TSLICE) {
    char command[256]; // Buffer to hold user input commands

    // Loop until the shared state indicates termination
    while (!shared_state->terminate) {
        printf("SimpleShell$ "); // Prompt the user for input
        // Read a line of input from stdin
        if (fgets(command, sizeof(command), stdin) == NULL) {
            continue;  // If there’s an input error, prompt again
        }

        // Tokenize the input command into a big args array
        char *args[20] = {0}; // Array to hold pointers to command arguments
        int arg_count = 0;    // Count of arguments parsed
        char *token = strtok(command, " \n"); // Split command by spaces and new lines

        // Loop to parse each token into the args array
        while (token != NULL && arg_count < 19) {
            args[arg_count++] = token; // Store the token in args
            token = strtok(NULL, " \n"); // Get the next token
        }
        args[arg_count] = NULL;  // Ensure the args array is NULL terminated

        // Check for the exit command to terminate the shell
        if (arg_count > 0 && strcmp(args[0], "exit") == 0) {
            printf("Exiting SimpleShell...\n"); // Notify user of exit
            shared_state->terminate = 1; // Set the termination flag
            print_stats(TSLICE);  // Print statistics before exiting
            break;  // Exit the loop and terminate the shell
        }

        // Check for a command separator `;` in the args array
        int split_index = -1; // Variable to hold the index of `;`
        for (int i = 0; i < arg_count; i++) {
            if (strcmp(args[i], ";") == 0) {
                split_index = i; // Store the index of `;`
                break;  // Exit the loop once `;` is found
            }
        }

        // If `;` is present, split the args array and execute each part separately
        if (split_index != -1) {
            args[split_index] = NULL;  // Terminate the first command at `;`
            handle_job_submission_2(args, &args[split_index + 1], TSLICE); // Handle both commands
        } else {
            // If there’s no `;`, handle the single command submission
            handle_job_submission(args, TSLICE); // Submit the single command for processing
        }
    }
}


int main(int argc, char *argv[]) {
    // Check if the correct number of command-line arguments is provided
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_cpus> <time_slice_in_ms>\n", argv[0]);
        return EXIT_FAILURE; // Exit with failure if incorrect usage
    }

    // Convert command-line arguments to integers for the number of CPUs and time slice
    int NCPU = atoi(argv[1]); // Number of CPUs
    int TSLICE = atoi(argv[2]); // Time slice in milliseconds

    // Create shared memory for the shared state structure
    int shm_fd_state = shm_open("/shared_state", O_CREAT | O_RDWR, 0666);
    if (shm_fd_state == -1) {
        perror("Shared memory creation failed for shared state"); // Handle error in shared memory creation
        exit(EXIT_FAILURE);
    }
    
    // Set the size of the shared memory segment to the size of SharedState
    ftruncate(shm_fd_state, sizeof(SharedState));
    
    // Map the shared memory segment to the address space of the process
    shared_state = mmap(0, sizeof(SharedState), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_state, 0);
    if (shared_state == MAP_FAILED) {
        perror("Shared memory mapping failed for shared state"); // Handle error in memory mapping
        exit(EXIT_FAILURE);
    }

    // Initialize the shared state structure
    shared_state->terminate = 0; // Set termination flag to false
    shared_state->job_count = 0;  // Initialize job count for queue 1
    shared_state->job_count2 = 0; // Initialize job count for queue 2
    shared_state->job_count3 = 0; // Initialize job count for queue 3
    shared_state->job_count4 = 0; // Initialize job count for queue 4

    // Create separate shared memory segments for each job queue
    int shm_fd1 = shm_open("/job_array1", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd1, sizeof(Job) * MAX_JOBS); // Set size for job array
    jobs = mmap(0, sizeof(Job) * MAX_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0); // Map job array to memory

    int shm_fd2 = shm_open("/job_array2", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd2, sizeof(Job) * MAX_JOBS);
    jobs2 = mmap(0, sizeof(Job) * MAX_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);

    int shm_fd3 = shm_open("/job_array3", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd3, sizeof(Job) * MAX_JOBS);
    jobs3 = mmap(0, sizeof(Job) * MAX_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd3, 0);

    int shm_fd4 = shm_open("/job_array4", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd4, sizeof(Job) * MAX_JOBS);
    jobs4 = mmap(0, sizeof(Job) * MAX_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd4, 0);

    // Check if any job arrays failed to map correctly
    if (jobs == MAP_FAILED || jobs2 == MAP_FAILED || jobs3 == MAP_FAILED || jobs4 == MAP_FAILED) {
        perror("Shared memory mapping failed"); // Handle mapping error
        exit(EXIT_FAILURE);
    }

    // Create a semaphore to manage job states, allowing synchronization between processes
    sem_job_state = sem_open("/sem_job_state", O_CREAT, 0644, 1);
    if (sem_job_state == SEM_FAILED) {
        perror("Semaphore creation failed"); // Handle semaphore creation error
        exit(EXIT_FAILURE);
    }

    // Set up signal handling for termination requests (e.g., Ctrl+C)
    signal(SIGINT, termination_request);

    // Fork a new process to start the scheduler
    pid_t scheduler_pid = fork();
    if (scheduler_pid == 0) { // In the child process
        scheduler(NCPU, TSLICE); // Call the scheduler function with the CPU count and time slice
        sem_close(sem_job_state); // Close the semaphore in the child process
        sem_unlink("/sem_job_state"); // Unlink the semaphore for cleanup

        // Clean up shared memory segments
        shm_unlink("/job_array1");
        shm_unlink("/job_array2");
        shm_unlink("/job_array3");
        shm_unlink("/job_array4");
        shm_unlink("/shared_state");

        // Unmap job arrays and shared state from the process's memory
        munmap(jobs, sizeof(Job) * MAX_JOBS);
        munmap(jobs2, sizeof(Job) * MAX_JOBS);
        munmap(jobs3, sizeof(Job) * MAX_JOBS);
        munmap(jobs4, sizeof(Job) * MAX_JOBS);
        munmap(shared_state, sizeof(SharedState));

        exit(0); // Exit child process after scheduler completes
    }

    // In the parent process, start the simple shell for user input
    simple_shell(NCPU, TSLICE);

    // After shell exits, signal the scheduler to terminate
    kill(scheduler_pid, SIGTERM); // Send termination signal to scheduler
    waitpid(scheduler_pid, NULL, 0); // Wait for the scheduler process to terminate

    // Clean up resources in the parent process
    sem_close(sem_job_state); // Close the semaphore
    sem_unlink("/sem_job_state"); // Unlink the semaphore

    // Unlink job arrays and shared state
    shm_unlink("/job_array1");
    shm_unlink("/job_array2");
    shm_unlink("/job_array3");
    shm_unlink("/job_array4");
    shm_unlink("/shared_state");

    // Unmap the job arrays and shared state from memory
    munmap(jobs, sizeof(Job) * MAX_JOBS);
    munmap(jobs2, sizeof(Job) * MAX_JOBS);
    munmap(jobs3, sizeof(Job) * MAX_JOBS);
    munmap(jobs4, sizeof(Job) * MAX_JOBS);
    munmap(shared_state, sizeof(SharedState));

    // Close the file descriptors for shared memory segments
    close(shm_fd1);
    close(shm_fd2);
    close(shm_fd3);
    close(shm_fd4);
    close(shm_fd_state);

    return EXIT_SUCCESS; // Exit the program successfully
}

