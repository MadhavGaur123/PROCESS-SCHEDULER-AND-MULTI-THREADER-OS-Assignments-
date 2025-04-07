#include "loader.h"
//These Definitons are already included in the loader.h -> elf.h I have written it here for my convinence.
//typedef struct {
    //unsigned char e_ident[EI_NIDENT]; 
    //unsigned short e_type;            
    //unsigned short e_machine;         
    //unsigned int e_version;           
    //unsigned int e_entry;            
    //unsigned int e_phoff;             
    //unsigned int e_shoff;             
    //unsigned int e_flags;            
    //unsigned short e_ehsize;          
    //unsigned short e_phentsize;       
    //unsigned short e_phnum;           
    //unsigned short e_shentsize;       
    //unsigned short e_shnum;           
    //unsigned short e_shstrndx;      
//} Elf32_Ehdr;
//typedef struct {
    //unsigned int p_type;             
    //unsigned int p_offset;           
   // unsigned int p_vaddr;            
    //unsigned int p_paddr;            
    //unsigned int p_filesz;           
    //unsigned int p_memsz;            
    //unsigned int p_flags;            
    //unsigned int p_align;            
//} Elf32_Phdr;
// typedef struct {
//     int si_signo;            // Signal number, specifies the signal causing the fault 
//     int si_errno;            // Error number associated with the signal
//     int si_code;             // Signal code
//     pid_t si_pid;            // Sending process ID, 
//     uid_t si_uid;            // User ID of the sending process, indicates the user ID of the sender
//     void *si_addr;           // Faulting address
//     int si_status;           
//     long si_band;            
//     union sigval si_value;   
// } siginfo_t;
// typedef struct {
//     void (*sa_handler)(int);         // Signal handling function,
//     void (*sa_sigaction)(int, siginfo_t *, void *); 
//     sigset_t sa_mask;                // blocking / non blocking signals
//     int sa_flags;                    
//     void (*sa_restorer)(void);       
// } struct sigaction;

// Global variables
Elf32_Ehdr *ehdr; //ehdr and phdr store pointers to ELF header and program headers.
Elf32_Phdr *phdr;
int fd; //for file handling
int numPageFaults = 0; //variable to store number of page faults
int numPageAllocations = 0; //variable to store number of page allocations
long long internalFragmentation = 0; //cumulative sum of all the internal fragmentation's

void loader_cleanup() { //memory free up once the program finished running it is a good practice to free up the memory that you have used
    if (ehdr != NULL) {
        free(ehdr); //cleaning up ehdr pointer
        ehdr = NULL;
    }
    if (phdr != NULL) {
        free(phdr); //cleaning up phdr pointer
        phdr = NULL;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

// Helper function to find the program header containing a given address
Elf32_Phdr* find_segment(unsigned long addr) {
    if (lseek(fd, ehdr->e_phoff, SEEK_SET) == -1) { // repositions the file offset to the start of the program header table.
        perror("Error seeking to program header table");
        return NULL;
    }

    Elf32_Phdr *current_phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr)); //current_phdr is dynamically allocated to temporarily hold each program header read from the ELF file.

    if (!current_phdr) { //error handling for memory allocating using malloc (sbrk and brk system call)
        perror("Failed to allocate memory for program header");
        return NULL;
    }

    for (int i = 0; i < ehdr->e_phnum; i++) { //The loop iterates over all program headers. ehdr->e_phnum gives the total number of program headers.
        if (read(fd, current_phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) { // reads the current phdr into the current_phdr memory location
            free(current_phdr);
            return NULL;
        }
        //to check weather the loaded segment contains the given segment. If it does then it returns the required phdr.
        if (current_phdr->p_type == PT_LOAD &&
            addr >= current_phdr->p_vaddr &&
            addr < current_phdr->p_vaddr + current_phdr->p_memsz) {
            return current_phdr;
        }
    }

    free(current_phdr); //after every iteration reset the variable value
    return NULL; //return NULL 
}

void sigsegv_handler(int signal, siginfo_t *info, void *context) {
    // Every time the code encounters memory that is yet to be allocated, it calls the custom SIGSEGV signal, hence increasing the number of page faults and the number of page allocations by 1.
    numPageFaults++;

    unsigned long fault_addr = (unsigned long)info->si_addr;
    unsigned long page_aligned_addr = (fault_addr / 4096) * 4096;

    // Find the segment containing the fault address
    Elf32_Phdr *segment = find_segment(fault_addr);
    if (!segment) {
        fprintf(stderr, "No valid segment found for address 0x%lx\n", fault_addr);
        exit(EXIT_FAILURE);
    }

    // Calculate the offset within the segment. For example, if a segment size > Page size, then only part of the segment will be loaded. 
    // This calculates how deep within the entire segment the new address is, as data before that will already be loaded.
    unsigned long segment_offset = page_aligned_addr - segment->p_vaddr;
    unsigned long aligned_file_offset = segment->p_offset + segment_offset; // Location of it from the start of the entire ELF file, i.e., 0x0000 (usually from the beginning of the ELF header)

    // Mapping the required page memory
    void *mapped_addr = mmap(
        (void *)page_aligned_addr,     // Address to start mapping
        4096,                          // Size of one page (4KB)
        PROT_READ | PROT_WRITE | PROT_EXEC, // Permissions
        MAP_PRIVATE | MAP_ANONYMOUS,   // Mapping flags
        -1,                            
        0                             
    );

    if (mapped_addr == MAP_FAILED) { // Error handling for memory that we have mapped to the page. If mapped_addr returns MAP_FAILED flag, the code exits with an error.
        perror("Failed to map memory page");
        exit(EXIT_FAILURE);
    }

    // Copy the segment data into the mapped memory from the file
    if (pread(fd, mapped_addr, 4096, aligned_file_offset) < 0) {
        perror("Failed to read segment data");
        munmap(mapped_addr, 4096); 
        exit(EXIT_FAILURE);
    }

    numPageAllocations++;
    // Calculate segment size
    unsigned long segment_size_bytes = segment->p_memsz;
    // Adding internal fragmentation if any during page allocation
    long long bytes_to_load = segment->p_memsz - segment_offset;
    long long bytesread;
    long long fragmentation;

    // If-else logic for calculating bytes read and internal fragmentation
    if (bytes_to_load < 4096) {
        bytesread = bytes_to_load;
        fragmentation = 4096 - bytes_to_load;
    } else {
        bytesread = 4096;
        fragmentation = 0;
    }

    internalFragmentation += fragmentation;
    

    //fprintf(stderr, "Address at which page fault occurred is: 0x%lx\n", (unsigned long)page_aligned_addr);
    //fprintf(stderr, "Segment info:\n");
    //fprintf(stderr, " start address of segment: 0x%lx\n", (unsigned long)segment->p_vaddr );
    //fprintf(stderr, " end address of segment: 0x%lx\n", (unsigned long)segment->p_vaddr + (unsigned long)segment->p_memsz);
     //fprintf(stderr, "  Segment size in bytes: %lu\n", segment_size_bytes);
    //printf("Current PageFault %d\n", numPageFaults);
    //printf("Page Number Allocated: %d\n", numPageAllocations);
    //printf("The Total Number of Available Bytes in 1 page: 4096\n");
    //printf("Bytes Read: %lld\n", bytesread);
    //printf("Internal fragmentation (in bytes) for the current page mapping: %lld\n", fragmentation);
    //printf("\n");

    free(segment);
}


void load_and_run_elf(char **exe) {
    // Open the ELF file
    fd = open(exe[1], O_RDONLY); // opening the required file
    if (fd < 0) { // error handling for file handling. If fd<0 ie invalid file descriptor.
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Allocate and read ELF header
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr)); //use of malloc call sbrk and brk system calls
    if (!ehdr) { //error handling for allocation of the elf header part
        perror("Failed to allocate memory for ELF header");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) { //loading the elf header into the ehdr variable
        perror("Failed to read ELF header");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for page faults. Basically we are creating a custom action for the sigsev signal(In built signal for loading the required page in memory during page fault)
    struct sigaction sa; 
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigsegv_handler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) { // error handling for the custom action that we have set up for the SIGSEV signal
        perror("Failed to set up signal handler");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Call the entry point
    Elf32_Addr entry_point = ehdr->e_entry; //Locate the entry point in the elf header. From this point the code should start running.
    int (*_start)() = (int (*)())(entry_point); //It casts the entry point address to the _start function pointer and executes it. In return we get the output of the code
    int result = _start();

    // Print required statistics
    printf("User _start return value = %d\n", result);
    printf("Number of page faults: %d\n", numPageFaults);
    printf("Number of page allocations: %d\n", numPageAllocations);
    printf("Internal fragmentation (in bytes): %lld\n", internalFragmentation);

    close(fd);
}

int main(int argc, char **argv) {
    if (argc != 2) { // error handling for arguments
        fprintf(stderr, "Usage: %s <ELF Executable>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    load_and_run_elf(argv); //calling the main function. Everything takes place inside this function
    loader_cleanup(); // for cleaning up the used memory
    return 0;
}
