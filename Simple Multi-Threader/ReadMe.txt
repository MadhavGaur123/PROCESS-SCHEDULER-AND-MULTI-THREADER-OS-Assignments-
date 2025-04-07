#OS 5

github link: https://github.com/henten28/OS5

Contributions: 
Sushen: 2D looping
Madhav: 1D looping

Basically the above code represents how threads are used. Threads is a concept used to compute two things parallely. 
In the above code we have computed matrix multiplication and vector addition using threads. 
The lambda function  represents a computational function to be executed for each index.
We have created two types of data structures do store from which index to which index a chunk is from and its respective lambda function. 
Basically what we are doing in the code is that (Explaining using Array Addition 9 elements and 3 threads)
1) We are dividing the problem into smaller sub problems ie chunks
2) We are creating a thread for each chunk which run parallely and execute the respective lambda function
3) For example in case of vector addition rather that creating a global variable and traversing through array for each element. In the above case we create three chunks 
chunk 1 will store elements from indicies 0 to 2 with lambda function TotalSum = TotalSum + i;
chunk 2 will store elements from indicies 3 to 5 with lambda function TotalSum = TotalSum + i;
chunk 3 will store elements from indicies 6 to 8 with lambda function TotalSum = TotalSum + i;
Now each of them will run simultaneously to avoid race condition we can use either semaphores, mutex or locks depending on how the global variable Total Sum is declared. 
This will reduce simultaneously as the sum of smaller sub-problems are calculated simultaneously. 
In the end all the children threads are joined to the parent thread the parent thread will keep on waiting until this happens. 
In the end the execution time is printed for each thread. 
Same can be applied to paralle function in which we use nested loops and call the lambda function for each (i,j) pair value in the iteration. 

Handling Non-Divisible Workloads
1-dimensional parallel_for:
a)chunk size is calculated by dividing the size by numThreads.
b)If size is not perfectly divisible by numThreads, some elements will be left unassigned after all threads are allocated chunk sizes.
c)A final additional thread (a) is created to handle any leftover elements.This thread is only created if there are leftover elements, ensuring no data is skipped.
2-dimensional parallel_for:
a)Similar logic is applied, where chunk1 and chunk2 are calculated based on the size of rows and columns divided by numThreads.
b)Any leftover rows are handled by an additional thread if the division isn't perfect.


1)Threads are created using pthread_create
2)All threads, including the additional one (if any), are joined using pthread_join


