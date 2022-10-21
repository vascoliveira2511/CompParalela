#include <stdio.h>
#include <stdlib.h>

#define N 10000000
#define K 4

void init(float *px, float *py, float *cx, float *cy)
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

int *kmeans(float *px, float *py, float *cx, float *cy)
{
    float *dx = (float *)malloc(N * sizeof(float));
    float *dy = (float *)malloc(N * sizeof(float));
    int *count = (int *)malloc(K * sizeof(int));
    float *sumx = (float *)malloc(K * sizeof(float));
    float *sumy = (float *)malloc(K * sizeof(float));

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sumx[i] = 0;
        sumy[i] = 0;
    }

    for (int i = 0; i < N; i++)
    {
        float min = 1000000;
        int min_index = 0;
        for (int j = 0; j < K; j++)
        {
            dx[i] = px[i] - cx[j];
            dy[i] = py[i] - cy[j];
            float dist = dx[i] * dx[i] + dy[i] * dy[i];
            if (dist < min)
            {
                min = dist;
                min_index = j;
            }
        }
        count[min_index]++;
        sumx[min_index] += px[i];
        sumy[min_index] += py[i];
    }

    for (int i = 0; i < K; i++)
    {
        cx[i] = sumx[i] / count[i];
        cy[i] = sumy[i] / count[i];
    }

    free(dx);
    free(dy);
    free(sumx);
    free(sumy);
    return count;
}

int has_converged(float *cx, float *cy, float *cx_old, float *cy_old)
{
    for (int i = 0; i < K; i++)
    {
        if (cx[i] != cx_old[i] || cy[i] != cy_old[i])
        {
            return 0;
        }
    }
    return 1;
}

int main()
{
    float *px = (float *)malloc(N * sizeof(float));
    float *py = (float *)malloc(N * sizeof(float));
    float *cx = (float *)malloc(K * sizeof(float));
    float *cy = (float *)malloc(K * sizeof(float));

    int iterator = 0;
    int *count;

    init(px, py, cx, cy);

    while (1)
    {
        float *cx_old = (float *)malloc(K * sizeof(float));
        float *cy_old = (float *)malloc(K * sizeof(float));

        for (int i = 0; i < K; i++)
        {
            cx_old[i] = cx[i];
            cy_old[i] = cy[i];
        }

        count = kmeans(px, py, cx, cy);

        if (has_converged(cx, cy, cx_old, cy_old))
        {
            break;
        }

        free(cx_old);
        free(cy_old);
        free(count);
        iterator++;
    }

    printf("Iterations: %d times \n ", iterator);
    for (int i = 0; i < K; i++)
    {
        printf("Cluster %d: (%f, %f) \n", i, cx[i], cy[i]);
        printf("Count: %d \n", count[i]);
    }

    free(px);
    free(py);
    free(cx);
    free(cy);

    return 0;
}