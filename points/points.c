#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#pragma pack(1)

typedef struct {
    int x, y, z;
    char valid;
} point_t;

void array_of_structs(char *input_file_path, int iterations) {
    FILE *input_file = fopen(input_file_path, "r");

    int length = 0;
    fscanf(input_file, "%d", &length);

    point_t *points = malloc(length * sizeof(point_t));
    if (points == NULL) {
        exit(1);
    }

    for (int i = 0; i < length; ++i) {
        fscanf(input_file, "%d %d %d %c", &points[i].x, &points[i].y, &points[i].z, &points[i].valid);
    }

    fclose(input_file);

    // calculate center
    clock_t start = clock();

    point_t center;
    for (int iteration = 0; iteration < iterations; ++iteration) {
        center.x = 0;
        center.y = 0;
        center.z = 0;
        center.valid = 1;

        for (int i = 0; i < length; ++i) {
            if (points[i].valid == '1') {
                center.x += points[i].x;
                center.y += points[i].y;
                center.z += points[i].z;
            }
        }
    }

    center.x /= length;
    center.y /= length;
    center.z /= length;


    clock_t end = clock();

    long double seconds = (long double) (end - start) / CLOCKS_PER_SEC;
    printf("%Lf\n", seconds);

    free(points);
}

typedef struct {
    int *x, *y, *z;
    char *valid;
} points_t;

void struct_of_arrays(char *input_file_path, int iterations) {
    FILE *input_file = fopen(input_file_path, "r");

    int length = 0;
    fscanf(input_file, "%d", &length);

    points_t points = {NULL, NULL, NULL, NULL};
    points.x = malloc(length * sizeof(int));
    points.y = malloc(length * sizeof(int));
    points.z = malloc(length * sizeof(int));
    points.valid = malloc(length * sizeof(char));

    if (points.x == NULL || points.y == NULL || points.z == NULL || points.valid == NULL) {
        exit(1);
    }

    for (int i = 0; i < length; ++i) {
        fscanf(input_file, "%d %d %d %c", &points.x[i], &points.y[i], &points.z[i], &points.valid[i]);
    }

    fclose(input_file);

    // calculate center
    clock_t start = clock();

    point_t center;
    for (int iteration = 0; iteration < iterations; ++iteration) {
        center.x = 0;
        center.y = 0;
        center.z = 0;
        center.valid = 1;

        for (int i = 0; i < length; ++i) {
            if (points.valid[i] == '1') {
                center.x += points.x[i];
                center.y += points.y[i];
                center.z += points.z[i];
            }
        }
    }

    center.x /= length;
    center.y /= length;
    center.z /= length;


    clock_t end = clock();

    long double seconds = (long double) (end - start) / CLOCKS_PER_SEC;
    printf("%Lf\n", seconds);

    free(points.x);
    free(points.y);
    free(points.z);
    free(points.valid);
}

int main(int argc, char **argv) {
    printf("%ld %ld\n", 3 * sizeof(int) + sizeof(char), sizeof(point_t));

    char *input_file_path = argv[1];
    int iterations = atoi(argv[2]);
    int version = atoi(argv[3]);

    switch (version) {
        case 0:
            array_of_structs(input_file_path, iterations);
            break;
        case 1:
            struct_of_arrays(input_file_path, iterations);
            break;
        default:
            break;
    }

    return 0;
}