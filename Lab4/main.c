#include <stdio.h>
#include <windows.h>

#define ARRAY_SIZE 100000
#define NUM_THREADS 3

typedef struct {
    int start_index;
    int end_index;
    int* array;
} ThreadData;

DWORD WINAPI calculate_power(LPVOID arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start_index; i <= data->end_index; ++i) {
        data->array[i] = data->array[i] * data->array[i]; 
    }
    return 0;
}

int main() {
    int array[ARRAY_SIZE];
    int arrayv2[ARRAY_SIZE];
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    HANDLE threads[NUM_THREADS]; 
    ThreadData thread_data[NUM_THREADS];
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = i + 1; 
        arrayv2[i] = i + 1;
    }

    //Multiple threads
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    int chunk_size = ARRAY_SIZE / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].end_index = (i + 1) * chunk_size - 1;
        thread_data[i].array = array;

        if (i == NUM_THREADS - 1) { 
            thread_data[i].end_index = ARRAY_SIZE - 1;
        }

        threads[i] = CreateThread(NULL, 0, calculate_power, &thread_data[i], 0, NULL);
    }

    QueryPerformanceCounter(&end);
    double elapsed_time_multithread = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    for (int i = 0; i < NUM_THREADS; ++i) {
        CloseHandle(threads[i]);
    }

    //Single thread
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    for(int i = 0; i < ARRAY_SIZE; i++){
        arrayv2[i] = arrayv2[i] * arrayv2[i];
    }
    QueryPerformanceCounter(&end);
    double elapsed_time_singlethread = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    printf("Time taken (Multi-threaded): %.9f seconds\n", elapsed_time_multithread);
    printf("Time taken (Single-threaded): %.9f seconds\n", elapsed_time_singlethread);

    return 0;
}
