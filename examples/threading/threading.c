#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Sleep for wait_to_obtain_ms milliseconds
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // Obtain the mutex
    pthread_mutex_lock(thread_func_args->mutex);
    printf("Mutex obtained by thread\n");

    // Hold the mutex for wait_to_release_ms milliseconds
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // Release the mutex
    pthread_mutex_unlock(thread_func_args->mutex);
    printf("Mutex released by thread\n");
    thread_func_args->thread_complete_success = true;
    // Free the dynamically allocated memory
    // free(thread_func_args);
    // pthread_exit(NULL);
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* thread_arg;
    thread_arg = (struct thread_data *)malloc(1*sizeof(struct thread_data ));
    if(thread_arg == NULL){
        printf("Unable to allocate memory for thread data\n");
        return false;
    }
    
    thread_arg->mutex = mutex;
    thread_arg->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_arg->wait_to_release_ms = wait_to_release_ms;
    int error; 
    error = pthread_create(thread, NULL, &threadfunc, (void *)thread_arg); 
    if (error != 0) {
        printf("\nThread can't be created : [%s]", strerror(error)); 
        free(thread_arg);
        return false;
    }
    return true;
}

