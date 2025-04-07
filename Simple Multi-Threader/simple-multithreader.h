#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h> //Memory allocation functions like malloc, calloc, and free
#include <cstring>// For String Operations
#include <sys/time.h>//For time Management, Chrono is also an option for this
using namespace std;

//taken directly from Sir's code, just changed it slightly 
typedef struct { //for running 1d
    int low; // starting value of vector array
    int high; //ending value of vector array
    function<void(int)>lambda; // represents a computational function to be executed for each index.
} thread_args;

typedef struct { // for runinng 2d
    int low1; // starting value of row
    int high1; //ending value of row
    int low2; // starting value of column
    int high2; //ending value of column
    function<void(int, int)> lambda; // represents a computational function to be executed for each index.
}thread_args_matrix;

void *thread_func(void *ptr) {// taken from Sir's sldies, slightly modified
    //error checking 
    if (ptr == nullptr) { 
        cout << "Error: Null pointer" << endl;
        exit(EXIT_FAILURE);
    }
    thread_args * t = ((thread_args*) ptr);
    for (int i = t->low; i < t->high; i++) {
        t->lambda(i);
    }
    return NULL;
}

void *thread_func_matrix(void*ptr){ // same as last fn just made it 2d
    //error checking 
    if (ptr == nullptr) {
        cout << "Error: Null pointer" << endl;
        exit(EXIT_FAILURE);
    }

    thread_args_matrix *t= (thread_args_matrix*) ptr; // Cast the pointer to `thread_args_matrix` to access the data.
    for(int i=t->low1; i<t->high1;i++){
        for(int j=t->low2;j<t->high2;j++){
            t->lambda(i,j);
        }
    }
    return NULL;
}

void parallel_for(int low, int high,function<void(int)> &&lambda, int numThreads){  // taken from the doc itself
        //error checking 
    if (high <= low) {
        cout << "Invalid Range. High <= Low" << endl;
        exit(EXIT_FAILURE);
    }
   
    numThreads--;
        if (numThreads <= 0) {
        cout << "Number of threads must be greater than 0." << endl;
        exit(EXIT_FAILURE);
    }

    //starting time calculation 
    struct timeval start, end;
    gettimeofday(&start, NULL);
    pthread_t tid[numThreads];
    thread_args args[numThreads]; //taken from Sir's slides
    int size=high-low;
    int chunk = (size)/numThreads;
   
    int y=numThreads;
   
    if((size)%numThreads!=0){
        y=chunk*numThreads;
    }
    //taken from Sir's slides, just added lambda
    for (int i=0; i<y;i++) {
        args[i].low=i*chunk;
        args[i].high=(i+1)*chunk;
        args[i].lambda=lambda;
        if (pthread_create(&tid[i],NULL,thread_func, (void*) &args[i]) != 0) {
            cout << "Failed to create thread " << i << endl;
            exit(EXIT_FAILURE);
        }
    }
   
    pthread_t a; //for the remaining ines 
    thread_args b; //for the remaining ones 
   
    if(y!=numThreads){
        b.high=high;
        b.low=y;
        b.lambda=lambda;
        if (pthread_create(&a, NULL, thread_func, (void*) &b) != 0) {
            cout << "Failed to create additional thread" << endl;
            exit(EXIT_FAILURE);
        }
    }
   
    for(int i=0;i<y;i++) {
        pthread_join(tid[i], NULL);
    }
    if(y!=numThreads){
        pthread_join(a, NULL);
    }

    //ending time
    gettimeofday(&end, NULL);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    cout<<"exec time "<<duration<<endl;
}

//same just made it 2d
// Parallel computation function for 2D arrays.
// Similar to the 1D version but processes a 2D range.
void parallel_for(int low1, int high1, int low2, int high2, function<void(int, int)> &&lambda, int numThreads){
        numThreads--;

    //starting time 
            if (numThreads <= 0) {
        cout << "Number of threads must be greater than 0." << endl;
        exit(EXIT_FAILURE);
    }


        struct timeval start, end;
        gettimeofday(&start, NULL);
       
        pthread_t tid[numThreads];
        thread_args_matrix args[numThreads];
        int size1=high1-low1;
        int size2= high2-low2;
        int chunk1 = (size1)/numThreads;
        int chunk2 = (size2)/numThreads;
       
        int y=numThreads;
       
        if(size1%numThreads!=0){
            y=chunk1*numThreads;
        }
       
        for(int i=0;i<y;i++){
            args[i].low1=i*chunk1;
            args[i].high1=(i+1)*chunk1;
            args[i].low2=low2;
            args[i].high2=high2;
            args[i].lambda=lambda;
            if (pthread_create(&tid[i], NULL, thread_func_matrix,(void*) &args[i]) != 0) {
                cout << " Failed to create thread " << endl;
                exit(EXIT_FAILURE);
            }
        }
       
        pthread_t a; //for the remaining ines 
        thread_args_matrix b; //for the remaining ines 
   
    if(y!=numThreads){
        b.high1=high1;
        b.low1=y;
        b.lambda=lambda;
        b.high2 = high2;
        b.low2 = low2;
        if (pthread_create(&a, NULL, thread_func_matrix, (void*) &b) != 0) {
            cout << "Failed to create additional thread " << endl;
            exit(EXIT_FAILURE);
        }
    }
       
    for(int i=0;i<y;i++) {
        if(pthread_join(tid[i], NULL)!=0){
          cout<<"Unable to join thread"<<endl;
          exit(EXIT_FAILURE);
        }
    }
    if(y!=numThreads){
        if(pthread_join(a, NULL)!=0){
          cout<<"Unable to join thread"<<endl;
          exit(EXIT_FAILURE);
        
        }
    }
        //ending time
    gettimeofday(&end, NULL);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    cout<<"exec time "<<duration<<endl;
}
int user_main(int argc, char **argv);
int main(int argc, char **argv) {
    int x = user_main(argc, argv);
    return x;
}
#define main user_main //taken directly from Sir's code

