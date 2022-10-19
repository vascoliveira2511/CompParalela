#include <stdio.h>
#include <stdlib.h>

#define N 10000000
#define K 4

void inicializar(float *px, float *py, float *cx, float *cy)
{
    srand(10);
    for (int i = 0; i < N; i++)
    {
        px[i] = (float)rand() / RAND_MAX;
        py[i] = (float)rand() / RAND_MAX;
    }

    for (int i = 0; i < K; i++)
    {
        cx[i] = px[i];
        cy[i] = py[i];
    }
}

int main()
{
    printf("N = %i, K = %i\n", N, K);
}