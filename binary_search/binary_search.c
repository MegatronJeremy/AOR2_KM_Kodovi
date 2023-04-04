#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int binary_search_v0(int *array, int length, int key) {
    int low = 0;
    int high = length - 1;

    while (low <= high) {
        int middle = (low + high) / 2;

        if (array[middle] == key) {
            return middle;
        } else if (array[middle] < key) {
            low = middle - 1;
        } else if (array[middle] > key) {
            high = middle + 1;
        }
    }

    return -1;
}

int binary_search_v1(int *array, int length, int key) {
    int low = 0;
    int high = length - 1;

    while (low <= high) {
        int middle = (low + high) / 2;

        int new_low = middle - 1;
        int new_high = middle + 1;

        // 0 - read prefetch, 1 - write prefetch
        // 3 - high temporal locality (necessary to leave data in cache)
        __builtin_prefetch(&array[(low + new_high) / 2], 0, 3);
        __builtin_prefetch(&array[(new_low + high) / 2], 0, 3);

        if (array[middle] == key) {
            return middle;
        } else if (array[middle] < key) {
            low = new_low;
        } else if (array[middle] > key) {
            high = new_high;
        }
    }

    return -1;
}

int main(int argc, char **argv) {
    int array_length = atoi(argv[1]);
    int keys_length = atoi(argv[2]);
    int iterations = atoi(argv[3]);


    int *array = malloc(array_length * sizeof(int));
    int *keys = malloc(keys_length * sizeof(int));

    if (array == NULL || keys == NULL) {
        exit(1);
    }

    for (int i = 0; i < array_length; ++i) {
        array[i] = i;
    }

    srand(time(NULL));
    for (int i = 0; i < keys_length; ++i) {
        keys[i] = rand() % array_length;
    }

    {
        clock_t start = clock();

        for (int iteration = 0; iteration < iterations; ++iteration) {
            for (int i = 0; i < keys_length; ++i) {
                int result = binary_search_v0(array, array_length, keys[i]);
            }
        }

        clock_t end = clock();
        double seconds = (end - start) / (double) CLOCKS_PER_SEC;
        printf("WITHOUT PREFETCH => %lf\n", seconds);
    }

    {
        clock_t start = clock();

        for (int iteration = 0; iteration < iterations; ++iteration) {
            for (int i = 0; i < keys_length; ++i) {
                int result = binary_search_v1(array, array_length, keys[i]);
            }
        }

        clock_t end = clock();
        double seconds = (end - start) / (double) CLOCKS_PER_SEC;
        printf("WITH PREFETCH => %lf\n", seconds);
    }

    free(array);
    free(keys);
}