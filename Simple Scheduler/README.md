Contributions: 
Sushen: Shared Memory creation, Structure creation, Error Handling, ReadMe
Madhav: Scheduler, Shell 

GitHub link: https://github.com/henten28/OS-Assignment-3-/tree/main

Explanation: 
First, We create a data structure called Job which contains: 
Process ID of the job
Flag indicating if the job is currently running
Flag indicating if the job is in the queue
Time when the job was submitted
Time when the job last started
Total wait time of the job
Total run time of the job
Time when the job was completed

Then we create another structure to hold shared state information which contains:
Count of jobs in priority 1
Count of jobs in priority 2
Count of jobs in priority 3
Count of jobs in priority 4

Then we create a current_time_ms function to get the exact time at which a function is called

Then we create a submit jobs function that works on the same logic and also contains implementation for each of the priorities individually, ie 1,2,3,4. submit_jobs works this way:
  we are using switch case for priorities
  then we have also implemented error handling for the queue being full or an error in forking
  then we initialise all variables of job and kill the program so that we can use round robin. 
  then once we break free from this loop we will also print the jobs currently in queue 

After the submit jobs we will have to create print_stats
  we will be iterating over all the job counts
  we will see in each of them if there is a job already in queue and then we will print the details of   it
  we have also used semaphore to avoid race condition

Next we have handle_job_submission (2), which basically reads user input 
  We have done error handling if the priority number specified is wrong or submit is not written or 3     things haven't been specified 
  If no error we will submit this job

Then we have is_process_terminated to see if a process has been terminated or not

Then we termination_request function which will handle ctrl+c signal request

Then we have scheduler function to manage job scheduling based on round robin strategy which entails:
  Loop until the shared state signals terminationand inside it we do:
     Wait for access to the shared job state semaphore
     Counter for the number of invoked jobs
     Array to store job index and its corresponding queue
     Process jobs in the 'jobs(1,2,3,4)' array
     then outside the loop, Release the semaphore after processing jobs
     Sleep for TSLICE milliseconds to allow job processing time
     and Process each invoked job based on the queue number in `num` which entails these steps:
       Get the job index
       Get the job queue number
       Handle job termination and completion for the 'jobs' array
       Similar logic is applied for jobs in the 'jobs2', 'jobs3', and 'jobs4' arrays



Then we have simple_shell Function to implement a simple shell interface for submitting jobswhich has
  Buffer to hold user input commands
  Loop until the shared state indicates termination which does the following
  Prompt the user for input
  Read a line of input from stdin
  If there’s an input error, prompt again
  Tokenize the input command into a big args array
  Array to hold pointers to command arguments
  Count of arguments parsed
  Split command by spaces and new lines
  Loop to parse each token into the args array
  Check for the exit command to terminate the shell
  Check for a command separator `;` in the args array
  If `;` is present, split the args array and execute each part separately

Then we have main which does the following: 
  Check if the correct number of command-line arguments is provided
  Convert command-line arguments to integers for the number of CPUs and time slice
  Create shared memory for the shared state structure
  Set the size of the shared memory segment to the size of SharedState
  Map the shared memory segment to the address space of the process
  Initialize the shared state structure
  Create separate shared memory segments for each job queue
  Check if any job arrays failed to map correctly
  Create a semaphore to manage job states, allowing synchronization between processes
  Set up signal handling for termination requests (e.g., Ctrl+C)
  Fork a new process to start the scheduler
  Clean up shared memory segments
  Unmap job arrays and shared state from the process's memory
  In the parent process, start the simple shell for user input
  After shell exits, signal the scheduler to terminate
  Clean up resources in the parent process
  Unlink job arrays and shared state
  Unmap the job arrays and shared state from memory
  Close the file descriptors for shared memory segments
  Exit the program successfully
     
     
