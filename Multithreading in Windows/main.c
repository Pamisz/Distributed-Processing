#include <stdio.h>
#include <windows.h>
#include <time.h>

#define ARRAY_SIZE 10000000
#define NUM_THREADS 6

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
    if (array == NULL) {
        printf("Failed to allocate memory. Exiting...\n");
        return 1;
    }
    
    for (int tn = 1; tn <= NUM_THREADS; tn++){
        for (int i = 0; i < ARRAY_SIZE; ++i) {
                array[i] = i + 1; 
        }
        HANDLE threads[tn]; 
        ThreadData thread_data[tn];
        int chunk_size = ARRAY_SIZE / tn;

        for (int i = 0; i < tn; ++i) {
            thread_data[i].start_index = i * chunk_size;
            thread_data[i].end_index = (i + 1) * chunk_size - 1;
            thread_data[i].array = array;

            if (i == tn - 1) { 
                thread_data[i].end_index = ARRAY_SIZE - 1;
            }
        }

        clock_t start = clock();
        for (int i = 0; i < tn; ++i) {
        threads[i] = CreateThread(NULL, 0, calculate, (void*)&thread_data[i], 0, NULL);
        if (threads[i] != INVALID_HANDLE_VALUE)
		{
			SetThreadPriority(threads[i], THREAD_PRIORITY_NORMAL);
		}
    }
    WaitForMultipleObjects(tn, threads, 1, INFINITE);
    clock_t end = clock();
	float elapsed_time_multithread = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken (%d-threaded): %f seconds\n", tn, elapsed_time_multithread);
    }
   
    free(array);

    return 0;
}
