#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void print_i386_cpuid_caches() {
    for (int id = 0;; id++) {
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
            break;
        }

        char *cache_type_string;
        switch (cache_type) {
            case 1:
                cache_type_string = "Data Cache";
                break;
            case 2:
                cache_type_string = "Instruction Cache";
                break;
            case 3:
                cache_type_string = "Unified Cache";
                break;
            default:
                cache_type_string = "Unknown Type Cache";
                break;
        }
        int cache_level = (eax >>= 5) & 0x7;
        int cache_is_self_initializing = (eax >>= 3) & 0x1; // does not need SW initialization
        int cache_is_fully_associative = (eax >>= 1) & 0x1;

        // See the page 3-192 of the manual.
        // ebx contains 3 integers of 10, 10 and 12 bits respectively
        unsigned int cache_sets = ecx + 1;
        unsigned int cache_coherency_line_size = (ebx & 0xFFF) + 1;
        unsigned int cache_physical_line_partitions = ((ebx >>= 12) & 0x3FF) + 1;
        unsigned int cache_ways_of_associativity = ((ebx >>= 10) & 0x3FF) + 1;

        // Total cache size is the product
        size_t cache_total_size =
                cache_ways_of_associativity * cache_physical_line_partitions * cache_coherency_line_size * cache_sets;

        printf(
                "Cache ID %d:\n"
                "- Level: %d\n"
                "- Type: %s\n"
                "- Sets: %d\n"
                "- System Coherency Line Size: %d bytes\n"
                "- Physical Line partitions: %d\n"
                "- Ways of associativity: %d\n"
                "- Total Size: %zu bytes (%zu kb)\n"
                "- Is fully associative: %s\n"
                "- Is Self Initializing: %s\n"
                "\n",
                id,
                cache_level,
                cache_type_string,
                cache_sets,
                cache_coherency_line_size,
                cache_physical_line_partitions,
                cache_ways_of_associativity,
                cache_total_size, cache_total_size >> 10,
                cache_is_fully_associative ? "true" : "false",
                cache_is_self_initializing ? "true" : "false"
        );
    }
}

int get_cache_line_size(int id) {
    return 64;
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

void write_matrix(FILE *output_file, int *matrix, int rows, int columns) {
    fprintf(output_file, "%d %d\n", rows, columns);
    for (int i = 0; i < rows; ++i) {
        if (i != 0) {
            fprintf(output_file, "\n");
        }

        for (int j = 0; j < columns; ++j) {
            if (j != 0) {
                fprintf(output_file, " ");
            }
            fprintf(output_file, "%d", matrix[i * columns + j]);
        }
    }
}

void rotate_matrix_v0(int *a, int n) {
    int m = n / 2;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            register int t = a[n * i + j];
            register int ni = i;
            register int nj = j;
            for (int k = 0; k < 4; k++) {
                register int ti = ni;
                ni = nj;
                nj = n - 1 - ti;
                register int old = a[n * ni + nj];
                a[n * ni + nj] = t;
                t = old;
            }
        }
    }
}

void rotate_matrix_v1(int *a, int n) {
    int m = n / 2;
    const int stride = 16;

    for (int ii = 0; ii < m; ii += stride) {
        for (int jj = 0; jj < m; jj += stride) {
            for (int i = ii; i < ii + stride && i < m; ++i) {
                for (int j = jj; j < jj + stride && j < m; ++j) {
                    register int t = a[n * i + j];
                    register int ni = i;
                    register int nj = j;
                    for (int k = 0; k < 4; k++) {
                        register int ti = ni;
                        ni = nj;
                        nj = n - 1 - ti;
                        register int old = a[n * ni + nj];
                        a[n * ni + nj] = t;
                        t = old;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {
//    print_i386_cpuid_caches();
//    return 0;

//    int line_size = get_cache_line_size(0);
//    printf("Cache 0 line size: %dB\n", line_size);
//    return 0;

    int version = atoi(argv[1]);
    int iterations = atoi(argv[2]);
    FILE *a_matrix_file = fopen(argv[3], "r");


    if (a_matrix_file == NULL) {
        exit(1);
    }


    int a_rows = 0, a_columns = 0;

    int *a = read_matrix(a_matrix_file, &a_rows, &a_columns);

    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        switch (version) {
            case 0:
                rotate_matrix_v0(a, a_rows);
                break;
            case 1:
                rotate_matrix_v1(a, a_rows);
                break;
            default:
                break;
        }
    }
    clock_t end = clock();

    double seconds = (end - start) * 1. / CLOCKS_PER_SEC;
    printf("%lfs\n", seconds);

//    FILE *b_matrix_file = fopen("out.txt", "w");
//    write_matrix(b_matrix_file, a, a_rows, a_columns);
//    fclose(b_matrix_file);

    fclose(a_matrix_file);
    free(a);

    return 0;
}