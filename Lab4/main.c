#include <stdio.h>
#include <windows.h>
#include <time.h>

#define ARRAY_SIZE 1000000
#define NUM_THREADS 3

typedef struct {
    int start_index;
    int end_index;
    long int* array;
} ThreadData;

DWORD WINAPI calculate(LPVOID arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start_index; i <= data->end_index; ++i) {
        for (int j = 0; j < 1000; j++) { 
            data->array[i]++;
        }
    }
    return 0;
}

int main() {
    long int* array = (long int*)malloc(ARRAY_SIZE * sizeof(long int)); 
    long int* arrayv2 = (long int*)malloc(ARRAY_SIZE * sizeof(long int)); 
    HANDLE threads[NUM_THREADS]; 
    ThreadData thread_data[NUM_THREADS];
    
    if (array == NULL || arrayv2 == NULL) {
        printf("Failed to allocate memory. Exiting...\n");
        return 1;
    }
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = i + 1; 
        arrayv2[i] = i + 1;
    }

    int chunk_size = ARRAY_SIZE / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].end_index = (i + 1) * chunk_size - 1;
        thread_data[i].array = array;

        if (i == NUM_THREADS - 1) { 
            thread_data[i].end_index = ARRAY_SIZE - 1;
        }
    }


    // Multiple threads
    clock_t start = clock();
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i] = CreateThread(NULL, 0, calculate, (void*)&thread_data[i], 0, NULL);
        if (threads[i] != INVALID_HANDLE_VALUE)
		{
			SetThreadPriority(threads[i], THREAD_PRIORITY_NORMAL);
		}
    }
    WaitForMultipleObjects(NUM_THREADS, threads, 1, INFINITE);
    clock_t end = clock();
	float elapsed_time_multithread = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken (Multi-threaded): %f seconds\n", elapsed_time_multithread);



    // Single thread
    start = clock();
    for(int i = 0; i < ARRAY_SIZE; i++){
       for (int j = 0; j < 1000; j++) { 
            arrayv2[i]++;
        }
    }
    end = clock();
    float elapsed_time_singlethread = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken (Single-threaded): %f seconds\n", elapsed_time_singlethread);


    free(array);
    free(arrayv2);

    return 0;
}
