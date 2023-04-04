#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void fun_v0(int **a, int **b, int **c, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
}

void fun_v1(int **a, int **b, int **c, int n)
{
    for (int i = 0; i < n; i++)
    {
        __builtin_prefetch(&a[i][0], 0, 3);
        __builtin_prefetch(&b[i][0], 0, 3);
        __builtin_prefetch(&c[i][0], 1, 3);

        int j;
        for (j = 0; j < n - 16; j += 16)
        {
            __builtin_prefetch(&a[i][j + 16], 0, 3);
            __builtin_prefetch(&b[i][j + 16], 0, 3);
            __builtin_prefetch(&c[i][j + 16], 1, 3);
            for (int k = j; k < j + 16; k++)
            {
                c[i][k] = a[i][k] + b[i][k];
            }
        }
        while (j < n)
        {
            c[i][j] = a[i][j] + b[i][j];
            j++;
        }
    }
}

int main(int argc, char **argv)
{
    int version = atoi(argv[1]);
    int iterations = atoi(argv[2]);

    int n = 8192;
    int **matA = (int **)malloc(n * sizeof(int *));
    int **matB = (int **)malloc(n * sizeof(int *));
    int **matC = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        matA[i] = (int *)malloc(n * sizeof(int));
        matB[i] = (int *)malloc(n * sizeof(int));
        matC[i] = (int *)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
            matA[i][j] = matB[i][j] % 100;
    }
    clock_t start = clock();

    for (int i = 0; i < iterations; i++)
    {
        switch (version)
        {
        case 0:
            fun_v0(matA, matB, matC, n);
            break;
        case 1:
            fun_v1(matA, matB, matC, n);
            break;
        default:
            break;
        }
    }

    clock_t end = clock();

    double seconds = (end - start) * 1. / CLOCKS_PER_SEC;

    printf("%lfs\n", seconds);

    for (int i = 0; i < n; i++)
    {
        free(matA[i]);
        free(matB[i]);
        free(matC[i]);
    }
    free(matA);
    free(matB);
    free(matC);
    return 0;
}