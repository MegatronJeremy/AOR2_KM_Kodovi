#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int get_cache_line_size(int id) {
    return 64; // byte line size
    // Variables to hold the contents of the 4 i386 legacy registers
    uint32_t eax = 4; // get cache info
    uint32_t ebx;
    uint32_t ecx = id; // cache id
    uint32_t edx;

    // generates output in 4 registers eax, ebx, ecx and edx
    __asm__ (
            "cpuid" // call i386 cpuid instruction
            :   "+a" (eax), // contains the cpuid command code, 4 for cache query
    "=b" (ebx),
    "+c" (ecx), // contains the cache id
    "=d" (edx)
            );

    // See the page 3-191 of the manual.
    int cache_type = eax & 0x1F;

    // end of valid cache identifiers
    if (cache_type == 0) {
        return -1;
    }

    return (ebx & 0xFFF) + 1;
}

int *read_matrix(FILE *input_file, int *rows, int *columns) {
    fscanf(input_file, "%d %d", rows, columns);

    int *matrix = malloc((*rows) * (*columns) * sizeof(int));

    if (matrix == NULL) {
        exit(3);
    }

    for (int i = 0; i < (*rows); ++i) {
        for (int j = 0; j < (*columns); ++j) {
            fscanf(input_file, "%d", &matrix[i * (*columns) + j]);
        }
    }

    return matrix;
}

void laplace_v0(int *previous, int *next, int rows, int columns) {
    for (int i = 1; i < (rows - 1); ++i) {
        for (int j = 1; j < (columns - 1); ++j) {
            int ic = (i + 0) * columns + (j + 0);
            int ip = (i - 1) * columns + (j + 0);
            int id = (i + 1) * columns + (j + 0);
            int il = (i + 0) * columns + (j - 1);
            int ir = (i + 0) * columns + (j + 1);

            int c = previous[ic];
            int u = previous[ip];
            int d = previous[id];
            int l = previous[il];
            int r = previous[ir];

            next[ic] = u + d + l + r - 4 * c;
        }
    }
}

void laplace_v1(int *previous, int *next, int rows, int columns) {
    const int stride = get_cache_line_size(0) / sizeof(int);

    for (int i_block = 1; i_block < (rows - 1); i_block += stride) {
        for (int j_block = 1; j_block < (columns - 1); j_block += stride) {
            for (int i = i_block; i < (i_block + stride) && i < (rows - 1); ++i) {
                for (int j = j_block; j < (j_block + stride) && j < (columns - 1); ++j) {
                    int ic = (i + 0) * columns + (j + 0);
                    int iu = (i - 1) * columns + (j + 0);
                    int id = (i + 1) * columns + (j + 0);
                    int il = (i + 0) * columns + (j - 1);
                    int ir = (i + 0) * columns + (j + 1);

                    int c = previous[ic];
                    int u = previous[iu];
                    int d = previous[id];
                    int l = previous[il];
                    int r = previous[ir];

                    next[ic] = u + d + l + r - 4 * c;
                }
            }
        }
    }
}

void laplace_v2(int *previous, int *next, int rows, int columns) {
    const int stride = get_cache_line_size(0) / sizeof(int);

    for (int i_block = 1; i_block < (rows - 1); i_block += stride) {
        for (int j_block = 1; j_block < (columns - 1); j_block += stride) {
            for (int i = i_block; i < (i_block + stride) && i < (rows - 1); ++i) {
                for (int j = j_block; j < (j_block + stride) && j < (columns - 1); j += 4) {
                    int ic0 = (i + 0) * columns + (j + 0) + 0;
                    int iu0 = (i - 1) * columns + (j + 0) + 0;
                    int id0 = (i + 1) * columns + (j + 0) + 0;
                    int il0 = (i + 0) * columns + (j - 1) + 0;
                    int ir0 = (i + 0) * columns + (j + 1) + 0;

                    int c0 = previous[ic0];
                    int u0 = previous[iu0];
                    int d0 = previous[id0];
                    int l0 = previous[il0];
                    int r0 = previous[ir0];

                    next[ic0] = u0 + d0 + l0 + r0 - 4 * c0;

                    int ic1 = (i + 0) * columns + (j + 0) + 1;
                    int iu1 = (i - 1) * columns + (j + 0) + 1;
                    int id1 = (i + 1) * columns + (j + 0) + 1;
                    int il1 = (i + 0) * columns + (j - 1) + 1;
                    int ir1 = (i + 0) * columns + (j + 1) + 1;

                    int c1 = previous[ic1];
                    int u1 = previous[iu1];
                    int d1 = previous[id1];
                    int l1 = previous[il1];
                    int r1 = previous[ir1];

                    next[ic1] = u1 + d1 + l1 + r1 - 4 * c1;

                    int ic2 = (i + 0) * columns + (j + 0) + 2;
                    int iu2 = (i - 1) * columns + (j + 0) + 2;
                    int id2 = (i + 1) * columns + (j + 0) + 2;
                    int il2 = (i + 0) * columns + (j - 1) + 2;
                    int ir2 = (i + 0) * columns + (j + 1) + 2;

                    int c2 = previous[ic2];
                    int u2 = previous[iu2];
                    int d2 = previous[id2];
                    int l2 = previous[il2];
                    int r2 = previous[ir2];

                    next[ic2] = u2 + d2 + l2 + r2 - 4 * c2;

                    int ic3 = (i + 0) * columns + (j + 0) + 3;
                    int iu3 = (i - 1) * columns + (j + 0) + 3;
                    int id3 = (i + 1) * columns + (j + 0) + 3;
                    int il3 = (i + 0) * columns + (j - 1) + 3;
                    int ir3 = (i + 0) * columns + (j + 1) + 3;

                    int c3 = previous[ic3];
                    int u3 = previous[iu3];
                    int d3 = previous[id3];
                    int l3 = previous[il3];
                    int r3 = previous[ir3];

                    next[ic3] = u3 + d3 + l3 + r3 - 4 * c3;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    FILE *matrix_file = fopen(argv[1], "r");
    int version = atoi(argv[2]);
    int iterations = atoi(argv[3]);

    if (matrix_file == NULL) {
        exit(1);
    }

    int rows = 0;
    int columns = 0;

    int *previous = read_matrix(matrix_file, &rows, &columns);
    int *next = malloc(rows * columns * sizeof(int));

    if (next == NULL) {
        exit(2);
    }

    clock_t start = clock();
    for (int i = 0; i < iterations; ++i) {
        switch (version) {
            case 0:
                laplace_v0(previous, next, rows, columns);
                break;
            case 1:
                laplace_v1(previous, next, rows, columns);
                break;
            case 2:
                laplace_v2(previous, next, rows, columns);
                break;
        }
    }
    clock_t end = clock();

    double seconds = (end - start) * 1. / CLOCKS_PER_SEC;
    printf("%lfs\n", seconds);

    fclose(matrix_file);
    free(previous);
    free(next);

    return 0;
}