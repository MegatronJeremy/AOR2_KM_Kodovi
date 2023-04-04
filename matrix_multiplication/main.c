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

void multiply_matrices_v0(int *a, int a_rows, int a_columns, int *b, int b_rows, int b_columns, int *c) {
    for (int i = 0; i < a_rows; ++i) {
        for (int j = 0; j < b_columns; ++j) {
            for (int k = 0; k < a_columns; ++k) {
                int a_index = i * a_columns + k;
                int b_index = k * b_columns + j;
                int c_index = i * b_columns + j;

                c[c_index] += a[a_index] * b[b_index];
            }
        }
    }
}

void multiply_matrices_v1(int *a, int a_rows, int a_columns, int *b, int b_rows, int b_columns, int *c) {
    const int block_size = get_cache_line_size(0);
    const int number_of_words = block_size / sizeof(int);

    for (int i = 0; i < a_rows; ++i) {
        for (int j = 0; j < b_columns; j += number_of_words) {
            for (int k = 0; k < a_columns; ++k) {
                for (int l = j; l < (j + number_of_words); ++l) {
                    int a_index = i * a_columns + k;
                    int b_index = k * b_columns + l;
                    int c_index = i * b_columns + l;

                    c[c_index] += a[a_index] * b[b_index];
                }
            }
        }
    }
}

void multiply_matrices_v2(int *a, int a_rows, int a_columns, int *b, int b_rows, int b_columns, int *c) {
    const int block_size = get_cache_line_size(0);
    const int number_of_words = block_size / sizeof(int);

    for (int i = 0; i < a_rows; i += number_of_words) {
        for (int j = 0; j < b_columns; j += number_of_words) {
            for (int k = 0; k < a_columns; ++k) {
                for (int m = i; m < (i + number_of_words); ++m) {
                    for (int l = j; l < (j + number_of_words); ++l) {
                        int a_index = m * a_columns + k; //[m][k]
                        int b_index = k * b_columns + l; //[k][l]
                        int c_index = m * b_columns + l; //[m][l]

                        c[c_index] += a[a_index] * b[b_index];
                    }
                }
            }
        }
    }
}

void multiply_matrices_v3(int *a, int a_rows, int a_columns, int *b, int b_rows, int b_columns, int *c) {
    int *b_trasposed = malloc(sizeof(int) * b_rows * b_columns);
    for (int i = 0; i < b_rows; ++i) {
        for (int j = 0; j < b_columns; ++j) {
            b_trasposed[j * b_columns + i] = b[i * b_columns + j];
        }
    }

    for (int i = 0; i < a_rows; ++i) {
        for (int j = 0; j < b_columns; ++j) {
            register int result = 0;

            for (int k = 0; k < a_columns; ++k) {
                int a_index = i * a_columns + k;
                int b_index = j * b_columns + k;

                result += a[a_index] * b_trasposed[b_index];
            }

            int c_index = i * b_columns + j;
            c[c_index] = result;
        }
    }

    free(b_trasposed);
}

void multiply_matrices_v4(int *a, int a_rows, int a_columns, int *b, int b_rows, int b_columns, int *c) {
    const int block_size = get_cache_line_size(0);
    const int number_of_words = block_size / sizeof(int);

    for (int jj = 0; jj < b_columns; jj += number_of_words) {
        for (int kk = 0; kk < a_columns; kk += number_of_words) {
            for (int i = 0; i < a_rows; i++) {
                for (int j = jj; j < (jj + number_of_words); j++) {
                    int c_index = i * b_columns + j; //[i][j]
                    register int sum = c[c_index];
                    for (int k = kk; k < (kk + number_of_words); k++) {
                        int a_index = i * a_columns + k; //[i][k]
                        int b_index = k * b_columns + j; //[k][j]

                        sum += a[a_index] * b[b_index];
                    }
                    c[c_index] = sum;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    // print_i386_cpuid_caches ( );
    // return 0;

    // int line_size = get_cache_line_size ( 0 );
    // printf ( "Cache 0 line size: %dB\n", line_size );

    int vers = atoi(argv[1]);
    int iter = atoi(argv[2]);
    FILE *a_matrix_file = fopen(argv[3], "r");
    FILE *b_matrix_file = fopen(argv[4], "r");
//    FILE *c_matrix_file = fopen(argv[5], "w");

    if (a_matrix_file == NULL || b_matrix_file == NULL) {
        exit(1);
    }


    int a_rows = 0, a_columns = 0;
    int b_rows = 0, b_columns = 0;

    int *a = read_matrix(a_matrix_file, &a_rows, &a_columns);
    int *b = read_matrix(b_matrix_file, &b_rows, &b_columns);

    int c_rows = a_rows;
    int c_columns = b_columns;
    int *c = calloc(c_rows * c_columns, sizeof(int));

    if (c == NULL) {
        exit(2);
    }

    clock_t start = clock();
    for (int i = 0; i < iter; i++) {
        switch (vers) {
            case 0:
                multiply_matrices_v0(a, a_rows, a_columns, b, b_rows, b_columns, c);
                break;
            case 1:
                multiply_matrices_v1(a, a_rows, a_columns, b, b_rows, b_columns, c);
                break;
            case 2:
                multiply_matrices_v2(a, a_rows, a_columns, b, b_rows, b_columns, c);
                break;
            case 3:
                multiply_matrices_v3(a, a_rows, a_columns, b, b_rows, b_columns, c);
                break;
            case 4:
                multiply_matrices_v4(a, a_rows, a_columns, b, b_rows, b_columns, c);
                break;
            default:
                break;
        }
    }
    clock_t end = clock();

    double seconds = (end - start) * 1. / CLOCKS_PER_SEC;
    printf("%lfs\n", seconds);

//     write_matrix ( c_matrix_file, c, c_rows, c_columns );

    fclose(a_matrix_file);
    fclose(b_matrix_file);
//    fclose(c_matrix_file);
    free(a);
    free(b);
    free(c);

    return 0;
}