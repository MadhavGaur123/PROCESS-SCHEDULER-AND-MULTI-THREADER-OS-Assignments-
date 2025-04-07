# OS4

github link:https://github.com/henten28/OS4
Contributions: 
Sushen: Segments, VM Parameters, ReadMe
Madhav: Page, Signals

in our loader.c we have first included loader.h, which is the same as that we used in Assignment 1

Then we created Ehdr structure (32), Phdr structure (32), siginfo_t, which includes 
  int for Signal number, specifies the signal causing the fault
  int for Error number associated with the signal
  int for Signal code
  pid for Sending process ID
  uid for User ID of the sending process, indicates the user ID of the sender
  void ptr for Faulting address
and sigaction which includes 
  Signal handling function
  blocking / non blocking signals

then we create some global variables 
  ehdr and phdr store pointers to ELF header and program headers
  variable to store number of page faults
  variable to store number of page allocations
  cumulative sum of all the internal fragmentation's

first method we have is 
  void loader_cleanup- memory free up once the program finished running it is a good practice to free up the memory that you have used

next up we have
  Elf32_Phdr* find_segment- Helper function to find the program header containing a given address
  repositions the file offset to the start of the program header table.
  current_phdr is dynamically allocated to temporarily hold each program header read from the ELF file.
  error handling for memory allocating using malloc (sbrk and brk system call)
  The loop iterates over all program headers. ehdr->e_phnum gives the total number of program headers.
  reads the current phdr into the current_phdr memory location
  check weather the loaded segment contains the given segment. If it does then it returns the required phdr.
  after every iteration reset the variable value
  return NULL


sigsegv_handler- Every time the code encounters memory that is yet to be allocated, it calls the custom SIGSEGV signal, hence increasing the number of page faults and the number of page allocations by 1.
  Find the segment containing the fault address
  Calculate the offset within the segment. For example, if a segment size > Page size, then only part of the segment will be loaded. 
  This calculates how deep within the entire segment the new address is, as data before that will already be loaded
  Mapping the required page memory
  Error handling for memory that we have mapped to the page. If mapped_addr returns MAP_FAILED flag, the code exits with an error.
  Copy the segment data into the mapped memory from the file
  Calculate segment size
  If-else logic for calculating bytes read and internal fragmentation
  


void load_and_run_elf which does the following: 
  Open the ELF file
  Allocate and read ELF header
  Set up signal handler for page faults. Basically we are creating a custom action for the sigsev signal(In built signal for loading the required page in memory during page fault)
  Call the entry point
  Locate the entry point in the elf header. From this point the code should start running.
  It casts the entry point address to the _start function pointer and executes it. In return we get the output of the code
  Print required statistics

  then main: which basically has two main statements
        load_and_run_elf(argv); //calling the main function. Everything takes place inside this function
    loader_cleanup(); // for cleaning up the used memory

  
