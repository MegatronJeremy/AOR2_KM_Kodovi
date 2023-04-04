//
// Created by xparh on 3/24/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int add_v0(int **x, int n)
{
    int t = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            t += x[i][j];
        }
    }
    return t;
}

int add_v1(int **x, int n)
{
    // kes linija - 64B - 64B / 4B = 16 podataka
    int t = 0;

    for (int i = 0; i < n - 1; i++)
    {
        __builtin_prefetch(&x[i][0], 0, 3);

        int j;
        for (j = 0; j < n - 16; j += 16)
        {
            __builtin_prefetch(&x[i][j + 16], 0, 3);
            for (int k = j; k < j + 16; k++)
            {
                t += x[i][k];
            }
        }
        while (j < n)
        {
            t += x[i][j++];
        }
    }

    return t;
}

int main(int argc, char **argv)
{
    int version = atoi(argv[1]);
    int iterations = atoi(argv[2]);

    int n = 8192;
    int result;
    int **mat = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        mat[i] = (int *)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
            mat[i][j] = j % 100;
    }
    clock_t start = clock();

    for (int i = 0; i < iterations; i++)
    {
        switch (version)
        {
        case 0:
            result = add_v0(mat, n);
            break;
        case 1:
            result = add_v1(mat, n);
            break;
        default:
            break;
        }
    }

    clock_t end = clock();

    double seconds = (end - start) * 1. / CLOCKS_PER_SEC;

    printf("%lfs\n", seconds);

    for (int i = 0; i < n; i++)
        free(mat[i]);
    free(mat);
    return 0;
}